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
// @author raver119@gmail.com
//

#ifndef LIBND4J_OPDESCRIPTOR_H
#define LIBND4J_OPDESCRIPTOR_H
#include <array/DataType.h>
#include <graph/scheme/node_generated.h>
#include <helpers/helper_hash.h>
#include <ops/InputType.h>

#include <initializer_list>
#include <string>
#include <vector>

namespace sd {
namespace ops {

/**
 *   This class is very basic info holder for ops. bean/pojo pretty much.
 *
 */
class SD_LIB_EXPORT OpDescriptor {
 protected:
  // opNum for legacy XYZ ops
  int _opNum = 0;

  // opName for CustomOp
  std::string _opName;

  // hash is used for ops lookup in OpRegistrator
  sd::LongType _hash = -1;

  // minimal required/expected number of inputs/outpus for this given op
  int _numInputs = 1;
  int _numOutputs = 1;

  // enum for ops. deprecated. will be removed
  sd::graph::OpClass _opClass;

  // special flag for divergent ops - ops that CAN and WILL modify graph behavior. Literally: IF, CASE.
  bool _divergent = false;

  // flag, if this given op allows in-place execution
  bool _allowsInplace = true;

  // minimal required number of T-type arguments.
  // -1 as value means: not limited, variable number of arguments
  int _tArgs = 0;

  // minimal required number of Integer-type arguments.
  // -1 as value means: not limited, variable number of arguments
  int _iArgs = 0;

  // field for BooleanOps
  bool _scalar = false;

  // field for LogicOps
  bool _logic = false;

  // default InputType is numeric
  InputType _inputType = InputType_NUMERIC;

  bool _sameMode = false;
  std::vector<sd::DataType> _allowedIns;
  std::vector<sd::DataType> _allowedOuts;

  // optional per-input configuration
  SD_MAP_IMPL<int, std::vector<sd::DataType>> _outputTypes;
  SD_MAP_IMPL<int, std::vector<sd::DataType>> _inputTypes;

  // field for ops that allow data type override at runtime
  bool _dtypeOverride = false;

  bool checkDataTypesMatch(sd::DataType needle, std::vector<sd::DataType>& haystack) const;

 public:
  // default constructor
  OpDescriptor(int numInputs, int numOutputs, std::string opName, bool allowsInplace);

  // constructor for boolean ops
  OpDescriptor(int numInputs, std::string opName, bool isScalar);
  OpDescriptor(int numInputs, const char* opName, bool isScalar);

  // default constructor
  OpDescriptor(int numInputs, int numOutputs, const char* opName, bool allowsInplace);

  // constructor for configurable op
  OpDescriptor(int numInputs, int numOutputs, const char* opName, bool allowsInplace, int tArgs, int iArgs);

  // constructor for non-configurable divergent op
  OpDescriptor(int numInputs, int numOutputs, std::string opName, bool allowsInplace, bool divergent);

  // constructor for non-configurable divergent op
  OpDescriptor(int numInputs, int numOutputs, const char* opName, bool allowsInplace, bool divergent);

  // constructor for configurable divergent op
  OpDescriptor(int numInputs, int numOutputs, const char* opName, bool allowsInplace, bool divergent, int tArgs,
               int iArgs);

  // constructor for logical ops (while, scope, etc)
  OpDescriptor(const char* opName, bool isLogic);

  bool operator==(const OpDescriptor& other) const;

  // default destructor
  ~OpDescriptor();

  // this method returns minimal expected number of T arguments
  int getNumberOfTArgs();

  // this method returns minimal expected number of Integer arguments
  int getNumberOfIArgs();

  // this method returns minimal expected number of inputs
  int getNumberOfInputs();

  // this method returns hash code for this operation
  sd::LongType getHash();

  // this method returns minimal expected number of outputs
  int getNumberOfOutputs();

  // this method returns opName (can be empty)
  std::string* getOpName();

  // returns TRUE if this op is divergent. FALSE otherwise
  bool isDivergent();

  // returns TRUE if this op allows in-place execution
  bool allowsInplace();

  // this method allows you to enable/disable inplace call for a given op
  void allowInplace(bool reallyAllow);

  // this method returns opNum (applicable for legacy XYZ ops only)
  int getOpNum();

  // this method allows to set specific opNum
  void setOpNum(int opNum);

  void setHash(sd::LongType hash);

  InputType inputType();

  OpDescriptor* setInputType(InputType type);
  OpDescriptor* setAllowedInputTypes(const std::initializer_list<sd::DataType>& dtype);
  OpDescriptor* setAllowedOutputTypes(const std::initializer_list<sd::DataType>& dtype);
  OpDescriptor* setAllowedInputTypes(int index, const std::vector<sd::DataType>& dtype);
  OpDescriptor* setAllowedOutputTypes(int index, const std::vector<sd::DataType>& dtype);
  OpDescriptor* setAllowedInputTypes(int index, sd::DataType dtype);
  OpDescriptor* setAllowedOutputTypes(int index, sd::DataType dtype);
  OpDescriptor* setAllowedInputTypes(sd::DataType dtype);
  OpDescriptor* setAllowedOutputTypes(sd::DataType dtype);
  OpDescriptor* allowOverride(bool reallyAllow);
  OpDescriptor* setSameMode(bool reallySame);
  OpDescriptor* setInputType(int idx, sd::DataType dtype);
  OpDescriptor* setOutputType(int idx, sd::DataType dtype);

  std::vector<sd::DataType> getOutputTypesForOutput(int index);
  std::vector<sd::DataType> getInputTypesForInput(int index);



  bool checkInputMatch(int index, sd::DataType dataType);
  bool checkOutputMatch(int index, sd::DataType dataType);
  bool isSameMode();

  bool isInherit(int index);
};
}  // namespace ops
}  // namespace sd

#endif  // LIBND4J_OPDESCRIPTOR_H
