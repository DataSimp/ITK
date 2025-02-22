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

#include "itkAnisotropicFourthOrderLevelSetImageFilter.h"
#include <iostream>

int
itkAnisotropicFourthOrderLevelSetImageFilterTest(int, char *[])
{
  using ImageType = itk::Image<float, 2>;
  using IndexType = ImageType::IndexType;

  auto im_init = ImageType::New();

  ImageType::RegionType r;
  ImageType::SizeType   sz = { { 128, 128 } };
  ImageType::IndexType  idx = { { 0, 0 } };
  r.SetSize(sz);
  r.SetIndex(idx);

  im_init->SetLargestPossibleRegion(r);
  im_init->SetBufferedRegion(r);
  im_init->SetRequestedRegion(r);
  im_init->Allocate();

  IndexType index;

  for (index[0] = 0; index[0] < 128; index[0]++)
    for (index[1] = 0; index[1] < 128; index[1]++)
    {
      if ((index[0] >= 32) && (index[0] <= 96) && (index[1] >= 32) && (index[1] <= 96))
      {
        im_init->SetPixel(index, static_cast<float>(-1));
      }
      else
      {
        im_init->SetPixel(index, static_cast<float>(1));
      }
    }

  using FilterType = itk::AnisotropicFourthOrderLevelSetImageFilter<ImageType, ImageType>;
  auto filter = FilterType::New();
  filter->SetMaxFilterIteration(2);
  filter->SetMaxNormalIteration(5);
  filter->SetNormalProcessConductance(0.5);

  filter->SetInput(im_init);
  filter->SetRMSChangeNormalProcessTrigger(0.1);
  std::cout << "max iteration = " << (filter->GetMaxFilterIteration()) << "\n";
  std::cout << "Starting processing.\n";
  filter->Update();
  filter->Print(std::cout);
  std::cout << "Passed.\n";
  return EXIT_SUCCESS;
}
