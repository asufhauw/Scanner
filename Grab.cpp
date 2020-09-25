// Grab.cpp
/*
    Note: Before getting started, Basler recommends reading the "Programmer's Guide" topic
    in the pylon C++ API documentation delivered with pylon.
    If you are upgrading to a higher major version of pylon, Basler also
    strongly recommends reading the "Migrating from Previous Versions" topic in the pylon C++ API documentation.

    This sample illustrates how to grab and process images using the CInstantCamera class.
    The images are grabbed and processed asynchronously, i.e.,
    while the application is processing a buffer, the acquisition of the next buffer is done
    in parallel.

    The CInstantCamera class uses a pool of buffers to retrieve image data
    from the camera device. Once a buffer is filled and ready,
    the buffer can be retrieved from the camera object for processing. The buffer
    and additional image data are collected in a grab result. The grab result is
    held by a smart pointer after retrieval. The buffer is automatically reused
    when explicitly released or when the smart pointer object is destroyed.
*/


#include <zxing/datamatrix/DataMatrixReader.h>
#include <zxing/datamatrix/detector/Detector.h>
#include <opencv2/opencv.hpp>
#include <zxing/datamatrix/DataMatrixReader.h>
#include <zxing/ReaderException.h>
#include <zxing/datamatrix/detector/Detector.h>
#include <zxing/common/GlobalHistogramBinarizer.h>
#include <zxing/common/HybridBinarizer.h>
#include <zxing/MatSource.h>
// Include files to use the pylon API.
#include <pylon/PylonIncludes.h>
//#include <opencv2/objdetect.hpp>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

// Namespace for using pylon objects.
using namespace Pylon;

// Namespace for using cout.
using namespace std;

// Number of images to be grabbed.
static const uint32_t numofimages = 100;

cv::Point toCvPoint(zxing::Ref<zxing::ResultPoint> resultPoint) {
    return cv::Point(resultPoint->getX(), resultPoint->getY());
}

string type2str(int type) {
    string r;

    uchar depth = type & CV_MAT_DEPTH_MASK;
    uchar chans = 1 + (type >> CV_CN_SHIFT);

    switch (depth) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
    }

    r += "C";
    r += (chans + '0');

    return r;
}

zxing::Ref<zxing::Result> decode(zxing::Ref<zxing::BinaryBitmap> image, zxing::DecodeHints hints) {
    (void)hints;
    zxing::datamatrix::Detector detector(image->getBlackMatrix());
    zxing::Ref<zxing::DetectorResult> detectorResult(detector.detect());
   
    zxing::ArrayRef<zxing::Ref<zxing::ResultPoint> > points(detectorResult->getPoints());

    zxing::datamatrix::Decoder decoder_;
    zxing::Ref<zxing::DecoderResult> decoderResult(decoder_.decode(detectorResult->getBits()));

    zxing::Ref<zxing::Result> result(
        new zxing::Result(decoderResult->getText(), decoderResult->getRawBytes(), points, zxing::BarcodeFormat::DATA_MATRIX));

    return result;
}
void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
    if (event == cv::EVENT_LBUTTONDOWN)
    {
        cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
    }
    else if (event == cv::EVENT_RBUTTONDOWN)
    {
        cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
    }
    else if (event == cv::EVENT_MBUTTONDOWN)
    {
        cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
    }
    else if (event == cv::EVENT_MOUSEMOVE)
    {
        
        try
        {
            cv::Mat img = *static_cast<cv::Mat*>(userdata);
            cout << type2str(img.depth()) << endl;
            try
            {
                cout << (int)img.at<cv::uint8_t>(cv::Point(x, y)) << endl;
            }
            catch (const std::exception&)
            {
                    
            }
          //  cout << "Mouse move over the window - position (" << x << ", " << y << ")" << img.at<ushort>(y,x) << endl;
        }
        catch (const std::exception& e)
        {
            cerr << e.what() << endl;
        }

    }
}

cv::Point startP1 = cv::Point(0, 0);
cv::Point endP2 = cv::Point(0, 0);
void getPoints(int event, int x, int y, int flags, void* userdata)
{
    if (event == cv::EVENT_LBUTTONDOWN)
    {
        cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
        startP1 = cv::Point(x, y);
    }
    else if (event == cv::EVENT_MOUSEMOVE)
    {
        cv::Mat img = *static_cast<cv::Mat*>(userdata);
        cout << type2str(img.depth()) << endl;

        cout << img.channels() << endl;
        try
        {
           // cv::circle(img, cv::Point(x, y), 2, cv::Scalar(150, 150, 150), 2, 8, 0);
            cout << img.at<cv::Vec3b>(cv::Point(x,y)) << endl;
            
        }
        catch (const std::exception&)
        {

        }
       //  cout << "Mouse move over the window - position (" << x << ", " << y << ")" << endl; //  << img->at<ushort>(y, x)
    }
    else if (event==cv::EVENT_LBUTTONUP)
    {
        cout << "Left button of the mouse is Up - position (" << x << ", " << y << ")" << endl;
        endP2 = cv::Point(x, y);
        cv::Mat* img = static_cast<cv::Mat*>(userdata);
        cv::Rect rect1 (startP1,endP2);
        *img = cv::Mat(*img, rect1);
    }
}




void decoder(cv::Mat image)
{
    // Create luminance  source
    try
    {
        while (cv::waitKey(10) <= 0)
            cv::imshow("window123", image);
        zxing::Ref<zxing::LuminanceSource> source = MatSource::create(image);
        zxing::Ref<zxing::Reader> reader;
        reader.reset(new zxing::datamatrix::DataMatrixReader);
        zxing::Ref<zxing::Binarizer> binarizer(new zxing::GlobalHistogramBinarizer(source));//HybridBinarizer GlobalHistogramBinarizer
        zxing::Ref<zxing::BinaryBitmap> bitmap(new zxing::BinaryBitmap(binarizer));
        zxing::DecodeHintType hint = zxing::DecodeHints::DATA_MATRIX_HINT | zxing::DecodeHints::TRYHARDER_HINT; // |  zxing::DecodeHints::TRYHARDER_HINT
        //cv::imshow("window", bitmap->getBlackMatrix());
        zxing::Ref<zxing::Result> result(reader->decode(bitmap, zxing::DecodeHints(hint)));

        // Get result point count
        int resultPointCount = result->getResultPoints()->size();

        for (int j = 0; j < resultPointCount; j++) {

            // Draw circle
            cv::circle(image, toCvPoint(result->getResultPoints()[j]), 0, cv::Scalar(110, 220, 0), 2);

        }

        // Draw boundary on image
        if (resultPointCount > 1) {
            for (int j = 0; j < resultPointCount; j++) {

                // Get start result point
                zxing::Ref<zxing::ResultPoint> previousResultPoint = (j > 0) ? result->getResultPoints()[j - 1] : result->getResultPoints()[resultPointCount - 1];

                // Draw line
                cv::line(image, toCvPoint(previousResultPoint), toCvPoint(result->getResultPoints()[j]), cv::Scalar(110, 220, 0), 2, 8);

                // Update previous point
                previousResultPoint = result->getResultPoints()[j];

            }

        }

        if (resultPointCount > 0) {
            // Draw text
            cv::putText(image, result->getText()->getText(), toCvPoint(result->getResultPoints()[0]), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(110, 220, 0));
            cout << result->getText()->getText() << endl;
        }

        cv::imshow("window", image);
        cv::waitKey(0);
    }
    catch (const zxing::ReaderException& e) {
        cerr << e.what() << " (ignoring)" << endl;

    }
    catch (const zxing::IllegalArgumentException& e) {
        cerr << e.what() << " (ignoring)" << endl;
    }
    catch (const zxing::Exception& e) {
        cerr << e.what() << " (ignoring)" << endl;
    }
    catch (const std::exception& e) {
        cerr << e.what() << " (ignoring)" << endl;
    }

}

void fixImage(cv::Mat img,cv::Mat oImg)
{
    // Find all contours in the image
    vector<vector<cv::Point> > contours;
    vector<cv::Vec4i> hierarchy;
    cv::Mat imgGray, imgthres;

    cv::cvtColor(img, imgGray, cv::COLOR_BGR2GRAY);

    cv::threshold(imgGray, imgthres, 70, 255, cv::THRESH_BINARY);

    while (cv::waitKey(10) <= 0)
        cv::imshow("window", imgGray);


    // Find all contours in the image
    findContours(imgthres, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::Point2f rect_points[4];
    cv::Mat boxPoints2f, boxPointsCov;
    cv::Rect rect;
    for (size_t i = 0; i < contours.size(); i++) {
        // Vertical rectangle
        if (contours[i].size() < 100) continue;

        rect = boundingRect(contours[i]);

        cv::Mat boxed = cv::Mat(imgGray, rect);

        while (cv::waitKey(10) <= 0)
            cv::imshow("window", boxed);
        cv::Point centerpoint = cv::Point(boxed.cols / 2, boxed.rows / 2);

        double angle = (int)minAreaRect(contours[i]).angle % 90;
        double scale = 1;
        cv::Mat rot_mat = getRotationMatrix2D(centerpoint, angle, scale);
        cv::Mat warp_rotate_dst;
        warpAffine(boxed, boxed, rot_mat, boxed.size());
        //cv::waitKey(0);
        cv::imshow("window", boxed);
        cv::waitKey(0);

        decoder(boxed);
        oImg = boxed.clone();
        //return;
    }
    decoder(imgGray);
    oImg = imgGray;
}

#include <pylon/BaslerUniversalInstantCamera.h>
int main(int argc, char* argv[])
{
    //cv::Mat testImg1 = cv::imread("C:\\Users\\dajo\\Projects\\Scanner\\datamatrix5.png");
    //testImg1 = fixImage(testImg1);
    //cv::imshow("window", testImg1);
    //cv::waitKey(0);

    //decoder(testImg1);
    // Before using any pylon methods, the pylon runtime must be initialized. 
    PylonInitialize();
    try
    {
       
        // Create an instant camera object with the camera device found first.
        CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());
        // Configure exposure time and mode

        // Print the model name of the camera.
     //   cout << "Using device " << camera.GetDeviceInfo().GetModelName() << endl;

        //Get a camera nodemap in order to access camera parameters
        GenApi::INodeMap& nodemap = camera.GetNodeMap();
        GenApi::CIntegerPtr width = nodemap.GetNode("Width");
        GenApi::CIntegerPtr height = nodemap.GetNode("Height");
        camera.Open();



        double d = CFloatParameter(nodemap, "ExposureTimeAbs").GetValue();
        // Set the exposure time mode to Standard
        // Note: Available on selected camera models only
        //CEnumParameter(nodemap, "ExposureTimeMode").SetValue("Standard");
        // Set the exposure time to 3500 microseconds
        CFloatParameter(nodemap, "ExposureTimeAbs").SetValue(3500.0);

        camera.MaxNumBuffer = 5;

        CImageFormatConverter formatConverter;

        formatConverter.OutputPixelFormat = PixelType_BGR8packed;

        CPylonImage pylonImage;

        int grabbedImages = 0;

        cv::Mat image;

        cv::Size framesize = cv::Size((int)width->GetValue(), (int)height->GetValue());

        camera.StartGrabbing(numofimages, GrabStrategy_LatestImageOnly);

        CGrabResultPtr ptrGrabResult;
        // Create a QRCodeDetector Object
        cv::namedWindow("window", 1);

        //set the callback function for any mouse event
        cv::setMouseCallback("window", getPoints, &image);
        while (camera.IsGrabbing())
        {
            camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);
            // do something ...
          
            if (ptrGrabResult->GrabSucceeded())
            {
              
                formatConverter.Convert(pylonImage, ptrGrabResult);
              
                image = cv::Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uint8_t*)pylonImage.GetBuffer());
               
                // set roi
                while(cv::waitKey(10)<= 0)
                    cv::imshow("window", image);

                // Fix the image
                fixImage(image, image);

                // show result image
            //    while (cv::waitKey(10) <= 0)
           //         cv::imshow("window", image);

                // Decode the image
             
            }
        }
    }
    catch (const GenericException & e)
    {
        cerr << "Failed to set the AOI width. Reason: "
            << e.GetDescription() << endl;
    }
    catch (const exception& ex)
    {
        cerr << "Failed to set the AOI width. Reason: "
            << ex.what() << endl;
    }
}
