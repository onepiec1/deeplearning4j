/* ******************************************************************************
 *
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License, Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0.
 *
 *  See the NOTICE file distributed with this work for additional
 *  information regarding copyright ownership.
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

//
// @author Yurii Shyrma (iuriish@yahoo.com), created on 19.02.2018
//

#include <system/op_boilerplate.h>
#if NOT_EXCLUDED(OP_maxpool3dnew)

#include <ops/declarable/CustomOperations.h>
#include <ops/declarable/helpers/convolutions.h>

namespace sd {
namespace ops {

//////////////////////////////////////////////////////////////////////////
CUSTOM_OP_IMPL(maxpool3dnew, 1, 1, false, 0, 14) {
  auto input = INPUT_VARIABLE(0);     // [bS, iD, iH, iW, iC] (NDHWC) or [bS, iC, iD, iH, iW] (NCDHW)
  auto output = OUTPUT_NULLIFIED(0);  // [bS, oD, oH, oW, iC] (NDHWC) or [bS, iC, oD, oH, oW] (NCDHW)

  int kD = INT_ARG(0);            // filter(kernel) depth
  int kH = INT_ARG(1);            // filter(kernel) height
  int kW = INT_ARG(2);            // filter(kernel) width
  int sD = INT_ARG(3);            // strides depth
  int sH = INT_ARG(4);            // strides height
  int sW = INT_ARG(5);            // strides width
  int pD = INT_ARG(6);            // paddings depth
  int pH = INT_ARG(7);            // paddings height
  int pW = INT_ARG(8);            // paddings width
  int dD = INT_ARG(9);            // dilations depth
  int dH = INT_ARG(10);           // dilations height
  int dW = INT_ARG(11);           // dilations width
  int isSameMode = INT_ARG(12);   // 1-SAME,  0-VALID
  int extraParam0 = INT_ARG(13);  // unnecessary for max case, required only for avg and pnorm cases
  int isNCDHW = block.getIArguments()->size() > 14 ? !INT_ARG(14) : 1;  // 1-NDHWC, 0-NCDHW

  REQUIRE_TRUE(input->rankOf() == 5, 0, "MAXPOOL3DNEW OP: rank of input array must be equal to 5, but got %i instead !",
               input->rankOf());
  REQUIRE_TRUE(dD != 0 && dH != 0 && dW != 0, 0,
               "MAXPOOL3DNEW op: dilation must not be zero, but got instead {%i, %i, %i}", dD, dH, dW);

  int bS, iC, iD, iH, iW, oC, oD, oH,
      oW;  // batch size, input channels, input depth/height/width, output channels, output depth/height/width;
  int indIOioC, indIOioD, indWoC, indWiC, indWkD;  // corresponding indexes
  ConvolutionUtils::getSizesAndIndexesConv3d(isNCDHW, 0, *input, *output, bS, iC, iD, iH, iW, oC, oD, oH, oW, indIOioC,
                                             indIOioD, indWiC, indWoC, indWkD);

  std::vector<sd::LongType> expectedOutputShape =
      ShapeUtils::composeShapeUsingDimsAndIdx({bS, iC, oD, oH, oW, 0, indIOioC, indIOioD, indIOioD + 1, indIOioD + 2});
  REQUIRE_TRUE(output->isSameShape(expectedOutputShape), 0,
               "MAXPOOL3D op: wrong shape of output array, expected is %s, but got %s instead !",
               ShapeUtils::shapeAsString(expectedOutputShape).c_str(), ShapeUtils::shapeAsString(output).c_str());
  // REQUIRE_TRUE(iD   >= kD && iH   >= kH && iW   >= kW, 0, "MAXPOOL3D OP: the input depth/height/width must be greater
  // or equal to kernel(filter) depth/height/width, but got [%i, %i, %i] and [%i, %i, %i] correspondingly !", iD,iH,iW,
  // kD,kH,kW); REQUIRE_TRUE(kD/2 >= pD && kH/2 >= pH && kW/2 >= pW, 0, "MAXPOOL3D OP: pad depth/height/width must not
  // be greater than half of kernel depth/height/width, but got [%i, %i, %i] and [%i, %i, %i] correspondingly !",
  // pD,pH,pW, kD,kH,kW);

  if (!isNCDHW) {
    input = new NDArray(input->permute({0, 4, 1, 2, 3}));    // [bS, iD, iH, iW, iC] -> [bS, iC, iD, iH, iW]
    output = new NDArray(output->permute({0, 4, 1, 2, 3}));  // [bS, oD, oH, oW, iC] -> [bS, iC, oD, oH, oW]
  }

  if (isSameMode)  // SAME
    ConvolutionUtils::calcPadding3D(pD, pH, pW, oD, oH, oW, iD, iH, iW, kD, kH, kW, sD, sH, sW, dD, dH, dW);

  ConvolutionUtils::pooling3d(block, *input, *output, kD, kH, kW, sD, sH, sW, pD, pH, pW, dD, dH, dW, 0, 1);

  if (!isNCDHW) {
    delete input;
    delete output;
  }

  return sd::Status::OK;
}

DECLARE_TYPES(maxpool3dnew) { getOpDescriptor()->setAllowedInputTypes(sd::DataType::ANY)->setSameMode(true); }

DECLARE_SHAPE_FN(maxpool3dnew) {
  int kD = INT_ARG(0);           // filter(kernel) depth
  int kH = INT_ARG(1);           // filter(kernel) height
  int kW = INT_ARG(2);           // filter(kernel) width
  int sD = INT_ARG(3);           // strides depth
  int sH = INT_ARG(4);           // strides height
  int sW = INT_ARG(5);           // strides width
  int pD = INT_ARG(6);           // paddings depth
  int pH = INT_ARG(7);           // paddings height
  int pW = INT_ARG(8);           // paddings width
  int dD = INT_ARG(9);           // dilations depth
  int dH = INT_ARG(10);          // dilations height
  int dW = INT_ARG(11);          // dilations width
  int isSameMode = INT_ARG(12);  // 1-SAME,  0-VALID
  // int extraParam0 = INT_ARG(13);
  int isNCDHW = block.getIArguments()->size() > 14 ? !INT_ARG(14) : 1;  // 1-NDHWC, 0-NCDHW

  REQUIRE_TRUE(dD != 0 && dH != 0 && dW != 0, 0,
               "MAXPOOL3DNEW op: dilation must not be zero, but got instead {%i, %i, %i}", dD, dH, dW);

  auto inputShapeInfo = inputShape->at(0);

  int idxID, idxIC;
  if (isNCDHW) {
    idxID = 2;
    idxIC = 1;
  } else {
    idxID = 1;
    idxIC = 4;
  }

  int bS = inputShapeInfo[1];          // batch size
  int iC = inputShapeInfo[idxIC + 1];  // input channels
  int iD = inputShapeInfo[idxID + 1];  // input depth
  int iH = inputShapeInfo[idxID + 2];  // input height
  int iW = inputShapeInfo[idxID + 3];  // input width

  int oD, oH, oW;  // output depth, height, width
  ConvolutionUtils::calcOutSizePool3D(oD, oH, oW, kD, kH, kW, sD, sH, sW, pD, pH, pW, dD, dH, dW, iD, iH, iW,
                                      isSameMode);

  sd::LongType outputShape[5];

  outputShape[0] = bS;
  if (isNCDHW) {
    outputShape[1] = iC;
    outputShape[2] = oD;
    outputShape[3] = oH;
    outputShape[4] = oW;
  } else {
    outputShape[1] = oD;
    outputShape[2] = oH;
    outputShape[3] = oW;
    outputShape[4] = iC;
  }

  auto desc = new ShapeDescriptor(ArrayOptions::dataType(inputShapeInfo), shape::order(inputShapeInfo), outputShape, 5);
  auto ret = SHAPELIST(ConstantShapeHelper::getInstance().createShapeInfo(desc));
  delete desc;
  return ret;
}

DECLARE_TYPES(maxpool3dnew_bp) {
  getOpDescriptor()->setAllowedInputTypes(sd::DataType::ANY)->setAllowedOutputTypes({ALL_FLOATS});
}

//////////////////////////////////////////////////////////////////////////
CUSTOM_OP_IMPL(maxpool3dnew_bp, 2, 1, false, 0, 14) {
  auto input = INPUT_VARIABLE(0);    // [bS, iD, iH, iW, iC] (NDHWC) or [bS, iC, iD, iH, iW] (NCDHW)
  auto gradO = INPUT_VARIABLE(1);    // [bS, oD, oH, oW, oC] (NDHWC) or [bS, oC, oD, oH, oW] (NCDHW), epsilon_next
  auto gradI = OUTPUT_NULLIFIED(0);  // [bS, iD, iH, iW, iC] (NDHWC) or [bS, iC, iD, iH, iW] (NCDHW), epsilon

  const int kD = INT_ARG(0);           // filter(kernel) depth
  const int kH = INT_ARG(1);           // filter(kernel) height
  const int kW = INT_ARG(2);           // filter(kernel) width
  const int sD = INT_ARG(3);           // strides depth
  const int sH = INT_ARG(4);           // strides height
  const int sW = INT_ARG(5);           // strides width
  int pD = INT_ARG(6);                 // paddings depth
  int pH = INT_ARG(7);                 // paddings height
  int pW = INT_ARG(8);                 // paddings width
  const int dD = INT_ARG(9);           // dilations depth
  const int dH = INT_ARG(10);          // dilations height
  const int dW = INT_ARG(11);          // dilations width
  const int isSameMode = INT_ARG(12);  // 1-SAME,  0-VALID
  int extraParam0 = INT_ARG(13);       // unnecessary for max case, required only for avg and pnorm cases
  int isNCDHW = block.getIArguments()->size() > 14 ? !INT_ARG(14) : 1;  // 1-NDHWC, 0-NCDHW

  REQUIRE_TRUE(input->rankOf() == 5, 0, "MAXPOOL3DNEW_BP op: input should have rank of 5, but got %i instead",
               input->rankOf());
  REQUIRE_TRUE(dD != 0 && dH != 0 && dW != 0, 0,
               "MAXPOOL3DNEW_BP op: dilation must not be zero, but got instead {%i, %i, %i}", dD, dH, dW);

  int bS, iC, iD, iH, iW, oC, oD, oH,
      oW;  // batch size, input channels, input depth/height/width, output channels, output depth/height/width;
  int indIOioC, indIOioD, indWoC, indWiC, indWkD;  // corresponding indexes
  ConvolutionUtils::getSizesAndIndexesConv3d(isNCDHW, 0, *input, *gradO, bS, iC, iD, iH, iW, oC, oD, oH, oW, indIOioC,
                                             indIOioD, indWiC, indWoC, indWkD);

  std::vector<sd::LongType> expectedGradOShape =
      ShapeUtils::composeShapeUsingDimsAndIdx({bS, iC, oD, oH, oW, 0, indIOioC, indIOioD, indIOioD + 1, indIOioD + 2});
  std::vector<sd::LongType> expectedGradIShape =
      ShapeUtils::composeShapeUsingDimsAndIdx({bS, iC, iD, iH, iW, 0, indIOioC, indIOioD, indIOioD + 1, indIOioD + 2});
  REQUIRE_TRUE(gradO->isSameShape(expectedGradOShape), 0,
               "MAXPOOL3DNEW_BP op: wrong shape of output's gradients array (next epsilon), expected is %s, but got %s "
               "instead !",
               ShapeUtils::shapeAsString(expectedGradOShape).c_str(), ShapeUtils::shapeAsString(gradO).c_str());
  REQUIRE_TRUE(
      gradI->isSameShape(expectedGradIShape), 0,
      "MAXPOOL3DNEW_BP op: wrong shape of input's gradients array (epsilon), expected is %s, but got %s instead !",
      ShapeUtils::shapeAsString(expectedGradIShape).c_str(), ShapeUtils::shapeAsString(gradI).c_str());

  if (!isNCDHW) {
    input = new NDArray(input->permute({0, 4, 1, 2, 3}));  // [bS, iD, iH, iW, iC] -> [bS, iC, iD, iH, iW]
    gradI = new NDArray(gradI->permute({0, 4, 1, 2, 3}));  // [bS, iD, iH, iW, iC] -> [bS, iC, iD, iH, iW]
    gradO = new NDArray(gradO->permute({0, 4, 1, 2, 3}));  // [bS, oD, oH, oW, iC] -> [bS, iC, oD, oH, oW]
  }

  if (isSameMode)  // SAME
    ConvolutionUtils::calcPadding3D(pD, pH, pW, oD, oH, oW, iD, iH, iW, kD, kH, kW, sD, sH, sW, dD, dH, dW);

  // NDArray<T> columnsWrongShape(input->ordering(), {bS, iC, oD, oH, oW, kD, kH, kW}, input->getWorkspace());
  // NDArray<T>* columns = columnsWrongShape.permute({0, 1, 5, 6, 7, 2, 3, 4});                      // [bS, iC, oD, oH,
  // oW, kD, kH, kW] -> [bS, iC, kD, kH, kW, oD, oH, oW]

  // ConvolutionUtils<T>::vol2col(*input, *columns, sD, sH, sW, pD, pH, pW, dD, dH, dW);                 // [bS, iC, iD,
  // iH, iW] is convoluted to [bS, iC, kD, kH, kW, oD, oH, oW]

  // NDArray<T>* columns2d = columnsWrongShape.reshape('c', {bS*iC*oD*oH*oW, kD*kH*kW});
  // NDArray<T>* gradOVector = gradO->reshape('c', {(int) gradO->lengthOf(), 1});
  // T extraParams[] = {(T)1., (T)1.};
  // columns2d->template applyTransform<simdOps::IsMax<T>>(extraParams);
  // columns2d->muliColumnVector(gradOVector);

  // ConvolutionUtils<T>::col2vol(*columns, *gradI, sD, sH, sW, pD, pH, pW, dD, dH, dW);                     // columns
  // [bS, iC, kD, kH, kW, oD, oH, oW] is de-convoluted to  [bS, iC, iD, iH, iW]

  // 0,1 - kernel Height/Width; 2,3 - stride Height/Width; 4,5 - pad Height/Width; 6,7 - dilation Height/Width; 8 -
  // poolingMode; 9 - unnecessary;
  ConvolutionUtils::pooling3dBP(block, *input, *gradO, *gradI, kD, kH, kW, sD, sH, sW, pD, pH, pW, dD, dH, dW, 0, 1);

  if (!isNCDHW) {
    delete input;
    delete gradI;
    delete gradO;
  }

  return sd::Status::OK;
}

DECLARE_SHAPE_FN(maxpool3dnew_bp) {
  auto desc = new  ShapeDescriptor(inputShape->at(0), ArrayOptions::dataType(inputShape->at(1)));
  return SHAPELIST(ConstantShapeHelper::getInstance().createShapeInfo(desc));
}

}  // namespace ops
}  // namespace sd

#endif
