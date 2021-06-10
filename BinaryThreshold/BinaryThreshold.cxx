#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImage.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryFillholeImageFilter.h"
#include "itkFlatStructuringElement.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkBinaryDilateImageFilter.h"

int
main(int argc, char * argv[])
{
  if (argc < 7)
  {
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << std::endl;
    std::cerr << " <InputImage> <OutputImage> <LowerThreshold>";
    std::cerr << " <UpperThreshold> <OutsideValue> <InsideValue>" << std::endl;
    return EXIT_FAILURE;
  }

  constexpr unsigned int Dimension = 2;
  using PixelType = unsigned char;

  const char * InputImage = argv[1];
  const char * OutputImage = argv[2];

  const auto LowerThreshold = static_cast<PixelType>(atoi(argv[3]));
  const auto UpperThreshold = static_cast<PixelType>(atoi(argv[4]));
  const auto OutsideValue = static_cast<PixelType>(atoi(argv[5]));
  const auto InsideValue = static_cast<PixelType>(atoi(argv[6]));

  using ImageType = itk::Image<PixelType, Dimension>;

  using ReaderType = itk::ImageFileReader<ImageType>;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(InputImage);

  //ãÐÖµ·Ö¸î
  using FilterType = itk::BinaryThresholdImageFilter<ImageType, ImageType>;
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput(reader->GetOutput());
  filter->SetLowerThreshold(LowerThreshold);
  filter->SetUpperThreshold(UpperThreshold);
  filter->SetOutsideValue(OutsideValue);
  filter->SetInsideValue(InsideValue);

  //ADD-BOX 
  //for (int i = 0; i < m_dims[1]; i++) {

  //    zsrc.at<signed short>(i, 0) = 0;
  //    zsrc.at<signed short>(i, m_dims[0] - 1) = 0;
  //}
  //for (int j = 0; j < m_dims[0]; j++) {
  //    zsrc.at<signed short>(0, j) = 0;
  //    zsrc.at<signed short>(m_dims[1] - 1, j) = 0;
  //}




  //Ìî³ä¿×¶´
  using FillholeType = itk::BinaryFillholeImageFilter<ImageType>;
  FillholeType::Pointer Fillhole = FillholeType::New();
  Fillhole->SetInput(filter->GetOutput());
  Fillhole->SetForegroundValue(InsideValue);

  Fillhole->Update();




  typename ImageType::SizeType size = Fillhole->GetOutput()->GetLargestPossibleRegion().GetSize();
  unsigned char* pSrc = Fillhole->GetOutput()->GetBufferPointer();

  for (int x = 0; x < size[0]; x++)
  {
      pSrc[x] = 1;
      pSrc[x * (size[1] - 1)] = 1;
  }

  for (int y = 0; y < size[1]; y++)
  {
      pSrc[y] = 1;
      pSrc[y * (size[0] - 1)] = 1;
  }

  //
  const unsigned int radiusValue = 1;
  using StructuringElementType = itk::FlatStructuringElement<Dimension>;
  StructuringElementType::RadiusType radius;
  radius.Fill(radiusValue);
  StructuringElementType structuringElement = StructuringElementType::Ball(radius);

  using BinaryErodeImageFilterType = itk::BinaryErodeImageFilter<ImageType, ImageType, StructuringElementType>;

  BinaryErodeImageFilterType::Pointer erodeFilter = BinaryErodeImageFilterType::New();
  erodeFilter->SetInput(Fillhole->GetOutput());
  erodeFilter->SetKernel(structuringElement);
  erodeFilter->SetForegroundValue(1); // Intensity value to erode
  erodeFilter->SetBackgroundValue(0);   // Replacement value for eroded voxels




  using BinaryDilateImageFilterType = itk::BinaryDilateImageFilter<ImageType, ImageType, StructuringElementType>;

  BinaryDilateImageFilterType::Pointer dilateFilter = BinaryDilateImageFilterType::New();
  dilateFilter->SetInput(erodeFilter->GetOutput());
  dilateFilter->SetKernel(structuringElement);
  dilateFilter->SetForegroundValue(1); // Value to dilate




  using WriterType = itk::ImageFileWriter<ImageType>;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(OutputImage);
  writer->SetInput(dilateFilter->GetOutput());


  try
  {
    writer->Update();
  }
  catch (itk::ExceptionObject & e)
  {
    std::cerr << "Error: " << e << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}