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
        cout << "Mouse move over the window - position (" << x << ", " << y << ")"  << endl;

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
       
         cout << "Mouse move over the window - position (" << x << ", " << y << ")" << endl; //  << img->at<ushort>(y, x)
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






int state = 0;
void decoder(cv::Mat image)
{

        
    // Create luminance  source
    try
    {
        zxing::Ref<zxing::LuminanceSource> source = MatSource::create(image);
        state = 1;
        zxing::Ref<zxing::Reader> reader;
        state = 2;
        reader.reset(new zxing::datamatrix::DataMatrixReader);
        state = 3;
        zxing::Ref<zxing::Binarizer> binarizer(new zxing::GlobalHistogramBinarizer(source));//HybridBinarizer GlobalHistogramBinarizer
        state = 4;
        zxing::Ref<zxing::BinaryBitmap> bitmap(new zxing::BinaryBitmap(binarizer)); 
        state = 5;
        zxing::DecodeHintType hint = zxing::DecodeHints::DATA_MATRIX_HINT | zxing::DecodeHints::TRYHARDER_HINT; // |  zxing::DecodeHints::TRYHARDER_HINT
        //cv::imshow("window", bitmap->getBlackMatrix());
        zxing::Ref<zxing::Result> result(reader->decode(bitmap, zxing::DecodeHints(hint)));
        state = 6;

        // Get result point count
        int resultPointCount = result->getResultPoints()->size();  
        state = 7;
        
        for (int j = 0; j < resultPointCount; j++) {

            // Draw circle
            cv::circle(image, toCvPoint(result->getResultPoints()[j]), 0, cv::Scalar(110, 220, 0), 2);

        }
        state = 8;

        // Draw boundary on image
        if (resultPointCount > 1) {
            state = 9;
            for (int j = 0; j < resultPointCount; j++) {

                // Get start result point
                zxing::Ref<zxing::ResultPoint> previousResultPoint = (j > 0) ? result->getResultPoints()[j - 1] : result->getResultPoints()[resultPointCount - 1];

                // Draw line
                cv::line(image, toCvPoint(previousResultPoint), toCvPoint(result->getResultPoints()[j]), cv::Scalar(110, 220, 0), 2, 8);

                // Update previous point
                previousResultPoint = result->getResultPoints()[j];

            }

        }
        state = 10;
        if (resultPointCount > 0) {
            state = 11;
            // Draw text
            cv::putText(image, result->getText()->getText(), toCvPoint(result->getResultPoints()[0]), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(110, 220, 0));
            cout << result->getText()->getText() << endl;
        }
       
        cv::imshow("window", image);
        cv::waitKey(0);
    }
    catch (const zxing::ReaderException& e) {
        cerr << e.what() << state << " (ignoring)" << endl;
      
    }
    catch (const zxing::IllegalArgumentException& e) {
        cerr << e.what() << state << " (ignoring)" << endl;
    }
    catch (const zxing::Exception& e) {
        cerr << e.what() << state << " (ignoring)" << endl;
    }
    catch (const std::exception& e) {
        cerr << e.what() << state << " (ignoring)" << endl;
    }

  
}


int main(int argc, char* argv[])
{
    std::cout << "HELLO WORD" << std::endl;
    // Find all contours in the image
    vector<vector<cv::Point> > contours;
    vector<cv::Vec4i> hierarchy;
    cv::Mat testImg, testImgGray, testImg1, testImg1Gray, testimgthreas;
    testImg = cv::imread("C:\\Users\\dajo\\Projects\\Scanner\\datamatrix10.png");
    cv::cvtColor(testImg, testImgGray, cv::COLOR_BGR2GRAY);
    cv::threshold(testImgGray, testimgthreas, 100, 255, cv::THRESH_BINARY);
    // Find all contours in the image
    findContours(testimgthreas, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::Point2f rect_points[4];
    cv::Mat boxPoints2f, boxPointsCov;
    cv::Rect rect;
    for (size_t i = 0; i < contours.size(); i++) {
        // Vertical rectangle
        if (contours[i].size() < 200) continue;

        rect = boundingRect(contours[i]);

        cv::Mat boxed = cv::Mat(testImgGray, rect);

        cv::Point centerpoint = cv::Point(boxed.cols/2,boxed.rows / 2);

        double angle = minAreaRect(contours[i]).angle;
        double scale = 1;
        cv::Mat rot_mat = getRotationMatrix2D(centerpoint, angle, scale);
        cv::Mat warp_rotate_dst;
        warpAffine(boxed, boxed, rot_mat, boxed.size());
        //cv::waitKey(0);
       // cv::imshow("test", boxed);

        //cv::waitKey(0);
        decoder(boxed);
    }
   
    // Draw all the contours
    //drawContours(testImg, contours, -1, cv::Scalar(0, 220, 0), 3);
    cv::imshow("test", testImg);
    cv::waitKey(0);
    decoder(testImg);
    testImg1 = cv::imread("C:\\Users\\dajo\\Projects\\Scanner\\datamatrix5.png");
    cv::cvtColor(testImg1, testImg1Gray, cv::COLOR_BGR2GRAY);
    findContours(testImg1Gray, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    // Draw all the contours
    drawContours(testImg1, contours, -1, cv::Scalar(0, 220, 0), 3);
    cv::imshow("test", testImg1);
    cv::waitKey(0);
    decoder(testImg1);
    // Before using any pylon methods, the pylon runtime must be initialized. 
    PylonInitialize();
    try
    {
        // Create an instant camera object with the camera device found first.
        CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());

        // Print the model name of the camera.
     //   cout << "Using device " << camera.GetDeviceInfo().GetModelName() << endl;

        //Get a camera nodemap in order to access camera parameters
        GenApi::INodeMap& nodemap = camera.GetNodeMap();

        camera.Open();

        GenApi::CIntegerPtr width = nodemap.GetNode("Width");
        GenApi::CIntegerPtr height = nodemap.GetNode("Height");

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
      
        cv::Mat grey;// = cv::Mat(image, 200, 200);


        cv::namedWindow("window", 1);

        //set the callback function for any mouse event
        cv::setMouseCallback("window", getPoints, &grey);
        while (camera.IsGrabbing())
        {
            camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);
            // do something ...
          
            if (ptrGrabResult->GrabSucceeded())
            {
              
                formatConverter.Convert(pylonImage, ptrGrabResult);
              
                image = cv::Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uint8_t*)pylonImage.GetBuffer());
               
                // set roi
               // cv::Mat imageRoi = cv::Mat(image, cv::Rect(cv::Point(739, 400), cv::Point(1060, 700)));
               
                cv::cvtColor(image, grey, cv::COLOR_BGR2GRAY);

                cv::rectangle(image, cv::Rect(cv::Point(739, 400), cv::Point(1060, 700)), cv::Scalar(0, 0, 0), 2, 8, 0);

                while(cv::waitKey(10)<= 0)
                    cv::imshow("window", grey);
                
                decoder(grey);
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
