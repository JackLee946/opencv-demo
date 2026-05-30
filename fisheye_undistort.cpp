/**
 * HF900 Fisheye Camera Undistortion Demo
 * Compatible: MSVC 2019+, GCC, Clang | OpenCV 4.x
 *
 * Build (Windows MSVC):
 *   Add this file to Visual Studio project, configure OpenCV include/lib paths
 *
 * Build (Linux/Mac):
 *   g++ fisheye_undistort.cpp -o fisheye_undistort \
 *       $(pkg-config --cflags --libs opencv4) -std=c++14
 *
 * Usage:
 *   fisheye_undistort              open default camera (index 0)
 *   fisheye_undistort 2            open camera index 2
 *   fisheye_undistort input.jpg    process single image
 *   fisheye_undistort input.mp4    process video file
 *   fisheye_undistort calib.yaml 0 load params from yaml + open camera
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

struct CalibParams {
    cv::Mat K;
    cv::Mat D;
    cv::Size imageSize;
};

// HF900 hardcoded calibration parameters
CalibParams getHF900Params() {
    CalibParams p;
    p.imageSize = cv::Size(1920, 1080);

    double kData[9] = {
        492.18903562,   0.0,          964.79922148,
          0.0,        493.09671636,   539.46187420,
          0.0,          0.0,            1.0
    };
    p.K = cv::Mat(3, 3, CV_64F, kData).clone();

    double dData[4] = { -0.13209352, 0.18585684, -0.26624088, 0.12803714 };
    p.D = cv::Mat(4, 1, CV_64F, dData).clone();

    return p;
}

// Load calibration parameters from YAML file
CalibParams loadCalibFromYaml(const std::string& path) {
    cv::FileStorage fs(path, cv::FileStorage::READ);
    if (!fs.isOpened()) {
        std::cerr << "Cannot open: " << path << ", using built-in params" << std::endl;
        return getHF900Params();
    }
    CalibParams p;
    int w = 0, h = 0;
    fs["image_width"]  >> w;
    fs["image_height"] >> h;
    p.imageSize = cv::Size(w, h);
    fs["camera_matrix"]           >> p.K;
    fs["distortion_coefficients"] >> p.D;
    fs.release();
    std::cout << "Loaded calibration from: " << path << std::endl;
    return p;
}

// Precomputed remap tables
struct UndistortMaps {
    cv::Mat map1, map2;
};

UndistortMaps buildMaps(const CalibParams& p, cv::Size outputSize) {
    UndistortMaps m;
    cv::fisheye::initUndistortRectifyMap(
        p.K, p.D,
        cv::Mat::eye(3, 3, CV_64F),
        p.K,
        outputSize,
        CV_16SC2,
        m.map1, m.map2
    );
    return m;
}

cv::Mat undistortFrame(const cv::Mat& src, const UndistortMaps& maps) {
    cv::Mat dst;
    cv::remap(src, dst, maps.map1, maps.map2, cv::INTER_LINEAR);
    return dst;
}

bool isImageFile(const std::string& s) {
    if (s.size() < 4) return false;
    std::string ext = s.substr(s.size() - 4);
    return (ext == ".jpg" || ext == ".png" || ext == ".bmp" || ext == ".tif");
}

bool isNumber(const std::string& s) {
    if (s.empty()) return false;
    for (size_t i = 0; i < s.size(); i++) {
        if (!isdigit((unsigned char)s[i])) return false;
    }
    return true;
}

void processImage(const std::string& path, const CalibParams& p) {
    cv::Mat src = cv::imread(path);
    if (src.empty()) {
        std::cerr << "Cannot read image: " << path << std::endl;
        return;
    }

    UndistortMaps maps = buildMaps(p, src.size());
    cv::Mat dst = undistortFrame(src, maps);

    cv::Mat display;
    cv::hconcat(src, dst, display);
    if (display.cols > 1920) {
        double scale = 1920.0 / display.cols;
        cv::resize(display, display, cv::Size(), scale, scale);
    }

    cv::imshow("Original | Undistorted", display);
    cv::imwrite("undistorted_output.jpg", dst);
    std::cout << "Saved: undistorted_output.jpg" << std::endl;
    std::cout << "Press any key to exit..." << std::endl;
    cv::waitKey(0);
}

void processStream(cv::VideoCapture& cap, const CalibParams& p) {
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) {
        std::cerr << "Cannot read frame" << std::endl;
        return;
    }

    cv::Size frameSize = frame.size();
    std::cout << "Resolution: " << frameSize.width << "x" << frameSize.height << std::endl;

    UndistortMaps maps = buildMaps(p, frameSize);

    bool showSplit = true;
    bool paused    = false;
    int  saveCount = 0;

    std::cout << "Keys: s=toggle split  p=pause  w=save frame  q=quit" << std::endl;

    while (true) {
        if (!paused) {
            cap >> frame;
            if (frame.empty()) break;
        }

        cv::Mat dst = undistortFrame(frame, maps);

        cv::Mat display;
        if (showSplit) {
            cv::hconcat(frame, dst, display);
            if (display.cols > 1920) {
                double scale = 1920.0 / display.cols;
                cv::resize(display, display, cv::Size(), scale, scale);
            }
            cv::putText(display, "Original",
                cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX,
                1.0, cv::Scalar(0, 255, 0), 2);
            cv::putText(display, "Undistorted",
                cv::Point(display.cols / 2 + 10, 30), cv::FONT_HERSHEY_SIMPLEX,
                1.0, cv::Scalar(0, 255, 0), 2);
        } else {
            display = dst;
        }

        cv::imshow("HF900 Undistort", display);

        int key = cv::waitKey(1) & 0xFF;
        if (key == 'q' || key == 27) {
            break;
        } else if (key == 's') {
            showSplit = !showSplit;
        } else if (key == 'p') {
            paused = !paused;
        } else if (key == 'w') {
            std::string fname = "frame_" + std::to_string(saveCount++) + ".jpg";
            cv::imwrite(fname, dst);
            std::cout << "Saved: " << fname << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    CalibParams params = getHF900Params();
    std::string source = "1";

    if (argc == 2) {
        source = argv[1];
    } else if (argc >= 3) {
        params = loadCalibFromYaml(argv[1]);
        source = argv[2];
    }

    std::cout << "K = " << std::endl << params.K << std::endl;
    std::cout << "D = " << params.D.t() << std::endl;

    if (isImageFile(source)) {
        processImage(source, params);
    } else {
        cv::VideoCapture cap;
        if (isNumber(source)) {
            int idx = atoi(source.c_str());
#ifdef _WIN32
            cap.open(idx, cv::CAP_DSHOW);
#else
            cap.open(idx, cv::CAP_V4L2);
#endif
            if (!cap.isOpened()) cap.open(idx);
            cap.set(cv::CAP_PROP_FRAME_WIDTH,  1920);
            cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
            std::cout << "Opened camera index=" << idx << std::endl;
        } else {
            cap.open(source);
            std::cout << "Opened video: " << source << std::endl;
        }

        if (!cap.isOpened()) {
            std::cerr << "Cannot open: " << source << std::endl;
            return -1;
        }

        processStream(cap, params);
        cap.release();
    }

    cv::destroyAllWindows();
    return 0;
}
