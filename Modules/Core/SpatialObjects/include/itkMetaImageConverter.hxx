/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkMetaImageConverter_hxx
#define itkMetaImageConverter_hxx

#include "itkImageRegionIterator.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkMath.h"

namespace itk
{
template <unsigned int VDimension, typename PixelType, typename TSpatialObjectType>
auto
MetaImageConverter<VDimension, PixelType, TSpatialObjectType>::CreateMetaObject() -> MetaObjectType *
{
  return dynamic_cast<MetaObjectType *>(new ImageMetaObjectType);
}

template <unsigned int VDimension, typename PixelType, typename TSpatialObjectType>
const char *
MetaImageConverter<VDimension, PixelType, TSpatialObjectType>::GetMetaObjectSubType()
{
  return "Image";
}

template <unsigned int VDimension, typename PixelType, typename TSpatialObjectType>
auto
MetaImageConverter<VDimension, PixelType, TSpatialObjectType>::AllocateImage(const ImageMetaObjectType * image) ->
  typename ImageType::Pointer
{
  auto rval = ImageType::New();

  using SizeType = typename ImageType::SizeType;
  using SpacingType = typename ImageType::SpacingType;
  using RegionType = typename ImageType::RegionType;

  SizeType    size;
  SpacingType spacing;

  for (unsigned int i = 0; i < VDimension; ++i)
  {
    size[i] = image->DimSize()[i];
    if (Math::ExactlyEquals(image->ElementSpacing()[i], NumericTraits<typename SpacingType::ValueType>::ZeroValue()))
    {
      spacing[i] = 1;
    }
    else
    {
      spacing[i] = image->ElementSpacing()[i];
    }
  }

  const RegionType region(size);
  rval->SetLargestPossibleRegion(region);
  rval->SetBufferedRegion(region);
  rval->SetRequestedRegion(region);
  rval->SetSpacing(spacing);
  rval->Allocate();
  return rval;
}

/** Convert a metaImage into an ImageMaskSpatialObject  */
template <unsigned int VDimension, typename PixelType, typename TSpatialObjectType>
auto
MetaImageConverter<VDimension, PixelType, TSpatialObjectType>::MetaObjectToSpatialObject(const MetaObjectType * mo)
  -> SpatialObjectPointer
{
  const auto * imageMO = dynamic_cast<const ImageMetaObjectType *>(mo);

  if (imageMO == nullptr)
  {
    itkExceptionMacro(<< "Can't convert MetaObject to MetaImage");
  }

  ImageSpatialObjectPointer imageSO = ImageSpatialObjectType::New();


  typename ImageType::Pointer myImage = this->AllocateImage(imageMO);

  itk::ImageRegionIteratorWithIndex<ImageType> it(myImage, myImage->GetLargestPossibleRegion());
  for (unsigned int i = 0; !it.IsAtEnd(); i++, ++it)
  {
    it.Set(static_cast<typename ImageType::PixelType>(imageMO->ElementData(i)));
  }

  imageSO->SetImage(myImage);
  imageSO->SetId(imageMO->ID());
  imageSO->SetParentId(imageMO->ParentID());
  imageSO->GetProperty().SetName(imageMO->Name());

  return imageSO.GetPointer();
}


/** Convert an Image SpatialObject into a metaImage */
template <unsigned int VDimension, typename PixelType, typename TSpatialObjectType>
auto
MetaImageConverter<VDimension, PixelType, TSpatialObjectType>::SpatialObjectToMetaObject(const SpatialObjectType * so)
  -> MetaObjectType *
{
  const ImageSpatialObjectConstPointer imageSO = dynamic_cast<const ImageSpatialObjectType *>(so);

  if (imageSO.IsNull())
  {
    itkExceptionMacro(<< "Can't downcast SpatialObject to ImageSpatialObject");
  }
  using ImageConstPointer = typename ImageType::ConstPointer;

  ImageConstPointer SOImage = imageSO->GetImage();

  float spacing[VDimension];
  int   size[VDimension];

  for (unsigned int i = 0; i < VDimension; ++i)
  {
    size[i] = SOImage->GetLargestPossibleRegion().GetSize()[i];
    spacing[i] = SOImage->GetSpacing()[i];
  }

  auto * imageMO = new MetaImage(VDimension, size, spacing, MET_GetPixelType(typeid(PixelType)));

  itk::ImageRegionConstIterator<ImageType> it(SOImage, SOImage->GetLargestPossibleRegion());
  for (unsigned int i = 0; !it.IsAtEnd(); i++, ++it)
  {
    imageMO->ElementData(i, it.Get());
  }

  imageMO->ID(imageSO->GetId());
  imageMO->BinaryData(true);

  imageMO->ElementDataFileName("LOCAL");

  imageMO->ObjectSubTypeName(this->GetMetaObjectSubType());

  if (this->GetWriteImagesInSeparateFile())
  {
    std::string filename = imageSO->GetProperty().GetName();
    if (filename.empty())
    {
      std::cout << "Error: you should set the image name when using"
                << " WriteImagesInSeparateFile." << std::endl;
      std::cout << "The image will be written locally." << std::endl;
    }
    else
    {
      filename += ".raw";
      imageMO->ElementDataFileName(filename.c_str());
    }
  }
  return imageMO;
}

} // end namespace itk

#endif
