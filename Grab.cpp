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


#include <opencv2/opencv.hpp>
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




#include <dmtx.h>

void decodeImg(cv::Mat img)
{

    DmtxImage* dImg = dmtxImageCreate(img.data, img.cols, img.rows, DmtxPack8bppK); //DmtxPack24bppRGB
    DmtxDecode* dDec = dmtxDecodeCreate(dImg, 1);
    std::string qrcode;
    cv::Mat outImg;
    cv::cvtColor(img, outImg, cv::COLOR_GRAY2BGR);
    DmtxTime timeout;
    timeout.sec = 4;
    timeout.usec = timeout.sec * 1000000;
    DmtxRegion* dReg = dmtxRegionFindNext(dDec, NULL);
    if (dReg != NULL)
    {
        DmtxMessage* dMsg = dmtxDecodeMatrixRegion(dDec, dReg, DmtxUndefined);
        if (dMsg != NULL)
        {
            qrcode = std::string((char*)dMsg->output);
            std::cout << qrcode << std::endl;
        }

        int height = dmtxDecodeGetProp(dDec, DmtxPropHeight);
        DmtxVector2 topLeft, topRight, bottomLeft, bottomRight;
        topLeft.X = 0;
        topLeft.Y = 0;

        topRight.X = 1.0;
        topRight.Y = 0;

        bottomRight.X = 1.0;
        bottomRight.Y = 1.0;

        bottomLeft.X = 0;
        bottomLeft.Y = 1.0;

        dmtxMatrix3VMultiplyBy(&topLeft, dReg->fit2raw);
        dmtxMatrix3VMultiplyBy(&topRight, dReg->fit2raw);
        dmtxMatrix3VMultiplyBy(&bottomLeft, dReg->fit2raw);
        dmtxMatrix3VMultiplyBy(&bottomRight, dReg->fit2raw);

        double rotate = (2 * M_PI) + atan2(topLeft.Y - bottomLeft.Y, bottomLeft.X - topLeft.X);
        int rotateInt = (int)(rotate * 180.0 / M_PI + 0.5);
        rotateInt = rotateInt >= 360 ? rotateInt - 360 : rotateInt;


        cv::Point p1(topLeft.X, height - 1 - topLeft.Y);
        cv::Point p2(topRight.X, height - 1 - topRight.Y);
        cv::Point p3(bottomRight.X, height - 1 - bottomRight.Y);
        cv::Point p4(bottomLeft.X, height - 1 - bottomLeft.Y);
        cv::line(outImg, cv::Point(p1.x, p1.y), cv::Point(p2.x, p2.y), cv::Scalar(0, 0, 255), 2);
        cv::line(outImg, cv::Point(p2.x, p2.y), cv::Point(p3.x, p3.y), cv::Scalar(0, 0, 255), 2);
        cv::line(outImg, cv::Point(p3.x, p3.y), cv::Point(p4.x, p4.y), cv::Scalar(0, 0, 255), 2);
        cv::line(outImg, cv::Point(p4.x, p4.y), cv::Point(p1.x, p1.y), cv::Scalar(0, 0, 255), 2);
        cv::imshow("DataMatrix", outImg);
        cv::waitKey(0);
    }


    dmtxDecodeDestroy(&dDec);
    dmtxImageDestroy(&dImg);

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
      //  cv::setMouseCallback("window", getPoints, &image);
        while (camera.IsGrabbing())
        {
            camera.RetrieveResult(1, ptrGrabResult, TimeoutHandling_ThrowException);
            // do something ...
          
            if (ptrGrabResult->GrabSucceeded())
            {
              
                formatConverter.Convert(pylonImage, ptrGrabResult);
              
                image = cv::Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uint8_t*)pylonImage.GetBuffer());
               
                // set roi
              //  while(cv::waitKey(10)<= 0)
              //      cv::imshow("window", image);

                // Fix the image
                cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
                cv::imshow("window", image);
                cv::waitKey(10);
                decodeImg(image);

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
