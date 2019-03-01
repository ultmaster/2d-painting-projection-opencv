#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <boost/progress.hpp>
#include "readply.hpp"

using namespace std;
using namespace cv;

vector<Point2f> get2DPoints(const vector<Point3f> &pts) {
    int sz = static_cast<int>(pts.size());
    Mat data_pts = Mat(sz, 3, CV_32F);
    for (int i = 0; i < data_pts.rows; i++) {
        data_pts.at<float>(i, 0) = pts[i].x;
        data_pts.at<float>(i, 1) = pts[i].y;
        data_pts.at<float>(i, 2) = pts[i].z;
    }
    PCA pca_analysis(data_pts, Mat(), PCA::DATA_AS_ROW);
//    for (int i = 0; i < 3; ++i) {
//        for (int j = 0; j < 3; ++j) {
//            cout << pca_analysis.eigenvectors.at<float>(i, j) << " ";
//        }
//        cout << endl;
//    }

    Mat transform(3, 2, CV_32F);
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 3; ++j) {
            transform.at<float>(j, i) = pca_analysis.eigenvectors.at<float>(i, j);
        }
    Mat mat_ret = data_pts * transform;
    vector<Point2f> ret(sz);
    for (int i = 0; i < 2; ++i) {
        float minx = 0;
        for (int j = 0; j < sz; ++j)
            minx = min(minx, mat_ret.at<float>(j, i));
        for (int j = 0; j < sz; ++j)
            mat_ret.at<float>(j, i) -= minx;
    }
    for (int i = 0; i < sz; ++i) {
        ret[i].x = mat_ret.at<float>(i, 0);
        ret[i].y = mat_ret.at<float>(i, 1);
    }
    return ret;
}

const int unitSize = 400;

int main(int argc, char **argv) {

    puts("Reading ply mesh...");
    ply::read_ply_file("scene_dense_mesh_refine_texture.ply");
    vector<Point3f> vertices;
    for (const auto &v: ply::vertices) {
        vertices.emplace_back(v.x, v.y, v.z);
    }
    puts("PCA running...");
    auto vertices2d = get2DPoints(vertices);
    int rows = 0, cols = 0;
    for (auto &v: vertices2d) {
        v.x *= unitSize;
        v.y *= unitSize;
        rows = max(rows, (int) ceil(v.y));
        cols = max(cols, (int) ceil(v.x));
    }
    Mat imgOut = Mat::zeros(rows, cols, CV_32FC3);
    Mat textureImage;
    imread("scene_dense_mesh_refine_texture.png").convertTo(textureImage, CV_32FC3, 1.0 / 255.0);

    puts("Texture running...");
    boost::progress_display show_progress(ply::face_count);
    for (const auto &face: ply::faces) {
        ++show_progress;
        int sz = 3;
        assert ((int) face.vertex_indices.size() == sz);
        assert ((int) face.texcoord.size() == sz * 2);
        vector<Point2f> triIn, triOut;
        for (int i = 0; i < sz; ++i) {
            triIn.emplace_back(face.texcoord[2 * i] * textureImage.rows, (1 - face.texcoord[2 * i + 1]) * textureImage.cols);
            triOut.push_back(vertices2d[face.vertex_indices[i]]);
        }
        Rect rectIn = boundingRect(triIn);
        Rect rectOut = boundingRect(triOut);
        vector<Point2f> triInCropped, triOutCropped;
        vector<Point> triOutCroppedInt;
        for (int i = 0; i < 3; ++i) {
            triInCropped.emplace_back(triIn[i].x - rectIn.x, triIn[i].y - rectIn.y);
            triOutCropped.emplace_back(triOut[i].x - rectOut.x, triOut[i].y - rectOut.y);
            triOutCroppedInt.emplace_back(round(triOutCropped.back().x), round(triOutCropped.back().y));
        }

        Mat imgInCropped;
        textureImage(rectIn).copyTo(imgInCropped);
        Mat warpMat = getAffineTransform(triInCropped, triOutCropped);
        Mat imgOutCropped = Mat::zeros(rectOut.height, rectOut.width, CV_32FC3);
        warpAffine(imgInCropped, imgOutCropped, warpMat, imgOutCropped.size(), INTER_LINEAR, BORDER_REFLECT_101);
        Mat mask = Mat::zeros(rectOut.height, rectOut.width, CV_32FC3);
        fillConvexPoly(mask, triOutCroppedInt, Scalar(1.0, 1.0, 1.0), 0, 0);
        multiply(imgOutCropped, mask, imgOutCropped);
        multiply(imgOut(rectOut), Scalar(1.0, 1.0, 1.0) - mask, imgOut(rectOut));
        imgOut(rectOut) = imgOut(rectOut) + imgOutCropped;
    }

    Mat imgWrite;
    imgOut.convertTo(imgWrite, CV_8UC3, 255.0);
    imwrite("output.png", imgWrite);
    puts("Done! Written to output.png.");
    return 0;
}
