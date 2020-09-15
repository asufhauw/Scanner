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

void display(cv::Mat& im, cv::Mat& bbox)
{
    int n = bbox.rows;
    for (int i = 0; i < n; i++)
    {
        line(im, 
            cv::Point2i(bbox.at<float>(i, 0), 
                bbox.at<float>(i, 1)), 
            cv::Point2i(bbox.at<float>((i + 1) % n, 0), 
                bbox.at<float>((i + 1) % n, 1)), 
            cv::Scalar(255, 0, 0), 3);
    }
    imshow("Result", im);
}
int main(int argc, char* argv[])
{
    // The exit code of the sample application.
    int exitCode = 0;

    // Before using any pylon methods, the pylon runtime must be initialized. 
    PylonInitialize();
    string testpath = "C:\\Users\\johan\\source\\repos\\Scanner\\qrcard.png";
    try
    {

        // Create an instant camera object with the camera device found first.
        CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());

        // Print the model name of the camera.
        cout << "Using device " << camera.GetDeviceInfo().GetModelName() << endl;

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
        cv::QRCodeDetector qrDecoder = cv::QRCodeDetector();

        while (camera.IsGrabbing())
        {
            camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);

            if (ptrGrabResult->GrabSucceeded())
            {
                cout << "SizeX: " << ptrGrabResult->GetWidth() << endl;
                cout << "SizeY: " << ptrGrabResult->GetHeight() << endl;

                formatConverter.Convert(pylonImage, ptrGrabResult);

                image = cv::Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uint8_t*)pylonImage.GetBuffer());
                
                cv::Mat bbox, rectifiedImage;


              
                // Detect QR Code in the Image
               // string opencvData;
                
                string opencvData = qrDecoder.detectAndDecode(image, bbox, rectifiedImage);
              //  string opencvData = "";
                // Check if a QR Code has been detected
                string strQrStatus;


                if (opencvData.length() > 0)
                {
                    cout << "Decoded Data : " << opencvData << endl;
                    display(image, bbox);
                    rectifiedImage.convertTo(rectifiedImage, CV_8UC3);
                    cv::imshow("Rectified QRCode", rectifiedImage);
                    //cv::waitKey(0);

                    cout << "QR Code Detected!" << endl;
                    cout << opencvData << endl;
                    strQrStatus = opencvData;

       
                    putText(image, strQrStatus, cv::Point(100, 100), cv::FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0, 255, 0), 2.0);
                    
                }
                else
                    strQrStatus = "QR Code NOT Detected";
               
                cv::imshow("window", image);
                cv::waitKey(10);
            }
        }
    }
    catch (const GenericException & e)
    {
        cout << "Failed to set the AOI width. Reason: "
            << e.GetDescription() << endl;
    }
    catch (const exception& ex)
    {
        ;
    }
}
