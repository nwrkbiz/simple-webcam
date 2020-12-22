/**
 * @file main.cpp
 * @brief Program entry point.
 * @author Daniel Giritzer
 * @copyright "THE BEER-WARE LICENSE" (Revision 42):
 * <giri@nwrk.biz> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Daniel Giritzer
 */

// FLTK
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Box.H>
#include <FL/fl_ask.H>

// std library
#include <memory>
#include <iostream>
#include <string>
#include <tuple>
#include <chrono>

// opencv
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

// boost
#include <boost/program_options.hpp>

#include "Icon.h" // icon for window manager (embedded into executable for maximum portability)

/**
 * Helper class to display opencv matrix containing a image data.
 */
class ImgShow {
public:

    enum fl_imgtype{
        grey =  1,
        greya = 2,
        rgb =   3,
        rgba =  4
    };

    /**
     * @param img OpenCV matrix containing image data
     * @param title Title to be shown in the window header
     * @param type Image type (use enum from this class namespace)
     * @param rescale if true min and max values of a given grayscale picture are rescaled between 0-255, defaults to false
     */
    ImgShow(const cv::Mat& img, const std::string& title, const ImgShow::fl_imgtype& type, bool rescale = false){

        m_Type = type;
        m_Rescale = rescale;
        m_Win = std::make_shared<Fl_Window>(img.cols,img.rows, title.c_str());
        m_Scr = std::make_shared<Fl_Scroll>(0,0,m_Win->w(), m_Win->h());
        m_Pic = std::make_shared<Fl_RGB_Image>(m_ImgRGB.data, img.cols, img.rows, type, 0);
        m_Box = std::make_shared<Fl_Box>(0,0, m_Pic->w(), m_Pic->h());
        m_Box->image(m_Pic.get());
        m_Icon = std::make_shared<Fl_RGB_Image>(icon_data, 500, 500, ImgShow::fl_imgtype::rgba, 0);
        m_Win->icon(m_Icon.get());
        m_Win->resizable(m_Box.get()); // some window managers may truncate big windows according their needs, so allow resizing here.
        m_Win->show();
        Update(img);
    }

    /**
     * Update the picture.
     * @param img OpenCV matrix containing image data
     */
    void Update(const cv::Mat& img){
        // Convert image data from BRG (opencv default) to RGB
        if(m_Type == fl_imgtype::rgb || m_Type == fl_imgtype::rgba){
            cv::cvtColor(img, m_ImgRGB, cv::COLOR_BGR2RGB);
        }
        else {
            if(m_Rescale){
                m_ImgRGB = img;
                cv::normalize(m_ImgRGB, m_ImgRGB, 0, 255, cv::NORM_MINMAX, CV_8U);
            }
            else
            {
                if(img.type() != CV_8U)
                    img.convertTo(m_ImgRGB, CV_8U);
                else
                    m_ImgRGB = img;
            }
        }
        m_Pic = std::make_shared<Fl_RGB_Image>(m_ImgRGB.data, m_ImgRGB.cols, m_ImgRGB.rows, m_Type, 0);
        m_Box->image(m_Pic.get());
        m_Box->redraw();
    }

    virtual ~ImgShow() = default;

private:
    std::shared_ptr<Fl_Window> m_Win;
    std::shared_ptr<Fl_Scroll> m_Scr;
    std::shared_ptr<Fl_RGB_Image> m_Pic;
    std::shared_ptr<Fl_RGB_Image> m_Icon;
    std::shared_ptr<Fl_Box> m_Box;
    cv::Mat m_ImgRGB;
    ImgShow::fl_imgtype m_Type;
    bool m_Rescale;
};


namespace po = boost::program_options;

int main(int argc, char** argv)
{

    // commandline options
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help", "Print help.")
    ("device", po::value<int>(), "Camara id to be used. (if not set a gui prompt will force you to set one)");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    // print help message
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return EXIT_SUCCESS;
    }

    int device = -1;
    if(vm.count("device")){
        device = vm["device"].as<int>();
    }

    std::shared_ptr<Fl_RGB_Image> icon = std::make_shared<Fl_RGB_Image>(icon_data, 500, 500, ImgShow::fl_imgtype::rgba, 0);
    if(device == -1){
        fl_message_hotspot(true); // popup msg box near mousepointer
        ((Fl_Double_Window*)fl_message_icon()->parent())->icon(icon.get());
        fl_message_title("Select camara to use");
        std::string deviceIdStr = fl_input("Camara device ID", "0");
        device = std::stoi(deviceIdStr);
    }

    cv::Mat frame = cv::Mat::zeros(5,5, CV_8U);
    cv::VideoCapture cap(device);

    if (!cap.isOpened()) {
        std::cerr << "ERROR! Unable to open camera\n";
        return EXIT_FAILURE;
    }

#ifdef WIN32
    FreeConsole(); // Get rid of windows console after parsing parameters
#endif

    // initial frame
    cap.read(frame);
    ImgShow vid{frame, "Video", ImgShow::fl_imgtype::rgb};

    // callback userdata
    std::tuple<cv::VideoCapture, ImgShow, cv::Mat> uData{ cap, vid, frame};

    // fltk idle callback, called periodically by fltk when ready
    Fl::add_idle([](void *p) -> void  {
            const std::tuple<cv::VideoCapture, ImgShow, cv::Mat> uData = *(static_cast<std::tuple<cv::VideoCapture, ImgShow, cv::Mat>*>(p));
            auto [cap, vid, frame] = uData;

            if (Fl::get_key('s')){ // save pic
                const auto now = std::chrono::system_clock::now();
                const size_t ts = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
                cv::imwrite(std::to_string(ts) + "-snap.png", frame);
            }

            cap.read(frame);
            vid.Update(frame);
        },

        // Data passed to callback
        &uData
    );

    return(Fl::run());
}
