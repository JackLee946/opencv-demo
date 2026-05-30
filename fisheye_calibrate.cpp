#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

int main(int argc, char** argv)
{
    cv::Size board_size(9, 6);
    float square_size = 25.0f;

    if (argc < 2) {
        std::cout << "Usage: fisheye_calibrate.exe <image1> <image2> ..." << std::endl;
        std::cout << "  e.g.  fisheye_calibrate.exe calib_img/*.jpg" << std::endl;
        std::cout << std::endl;
        std::cout << "Parameters (edit source to change):" << std::endl;
        std::cout << "  checkerboard inner corners: " << board_size.width << " x " << board_size.height << std::endl;
        std::cout << "  square size: " << square_size << " mm" << std::endl;
        return -1;
    }

    std::vector<std::vector<cv::Point3f>> object_points;
    std::vector<std::vector<cv::Point2f>> image_points;

    std::vector<cv::Point3f> obj;
    for (int i = 0; i < board_size.height; ++i) {
        for (int j = 0; j < board_size.width; ++j) {
            obj.push_back(cv::Point3f(j * square_size, i * square_size, 0.0f));
        }
    }

    cv::Size image_size;
    int success_count = 0;

    for (int i = 1; i < argc; ++i) {
        cv::Mat img = cv::imread(argv[i], cv::IMREAD_GRAYSCALE);
        if (img.empty()) {
            std::cerr << "Failed to load: " << argv[i] << std::endl;
            continue;
        }

        image_size = img.size();

        std::vector<cv::Point2f> corners;
        bool found = cv::findChessboardCorners(img, board_size, corners,
            cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE);

        if (found) {
            cv::cornerSubPix(img, corners, cv::Size(11, 11), cv::Size(-1, -1),
                cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.01));

            object_points.push_back(obj);
            image_points.push_back(corners);
            ++success_count;

            cv::drawChessboardCorners(img, board_size, corners, found);
            std::cout << "[" << success_count << "] OK: " << argv[i] << std::endl;
        } else {
            std::cerr << "FAIL: " << argv[i] << " (no corners found)" << std::endl;
        }

        cv::Mat display;
        cv::cvtColor(img, display, cv::COLOR_GRAY2BGR);
        cv::resize(display, display, cv::Size(800, 600));
        cv::imshow("Calibration", display);
        if (cv::waitKey(100) == 27) break;
    }

    if (image_points.size() < 5) {
        std::cerr << "Need at least 5 valid images, got: " << image_points.size() << std::endl;
        return -1;
    }

    cv::Mat K = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat D = cv::Mat::zeros(4, 1, CV_64F);
    std::vector<cv::Mat> rvecs, tvecs;

    int flags = cv::fisheye::CALIB_RECOMPUTE_EXTRINSIC
              | cv::fisheye::CALIB_CHECK_COND
              | cv::fisheye::CALIB_FIX_SKEW;

    double rms = cv::fisheye::calibrate(object_points, image_points, image_size,
        K, D, rvecs, tvecs, flags,
        cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 100, 1e-6));

    std::cout << "========================================" << std::endl;
    std::cout << "Calibration finished" << std::endl;
    std::cout << "  RMS reprojection error: " << rms << std::endl;
    std::cout << "  Valid images: " << success_count << " / " << (argc - 1) << std::endl;
    std::cout << "  Image size: " << image_size.width << " x " << image_size.height << std::endl;
    std::cout << "  K = " << K << std::endl;
    std::cout << "  D = " << D << std::endl;

    std::string out_file = "fisheye_intrinsics.yaml";
    cv::FileStorage fs(out_file, cv::FileStorage::WRITE);
    fs << "image_width" << image_size.width;
    fs << "image_height" << image_size.height;
    fs << "K" << K;
    fs << "D" << D;
    fs.release();
    std::cout << "Saved to: " << out_file << std::endl;

    cv::destroyAllWindows();
    return 0;
}