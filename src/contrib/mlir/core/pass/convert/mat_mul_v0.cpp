//*****************************************************************************
// Copyright 2017-2020 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//*****************************************************************************

#include "contrib/mlir/core/ngraph_dialect/ops.hpp"
#include "contrib/mlir/core/pass/ng_dialect_builder.hpp"
#include "ngraph/ops.hpp"

mlir::Operation*
    ngraph::pass::NgDialectConversionPass::createOp(NgDialectConversionPass& NgDialectObj,
                                                    const ngraph::op::v0::MatMul* ngNode)
{
    auto matmulNode = dynamic_cast<const ngraph::op::v0::MatMul*>(ngNode);
    NGRAPH_CHECK(
        ngNode, matmulNode != nullptr, "ngNode ", ngNode->description(), " is not a v0::MatMul");

    auto op = NgDialectObj.createGenericOp<mlir::NGMatMulOp>(ngNode);
    auto matmulOp = llvm::cast<mlir::NGMatMulOp>(op);
    matmulOp.setTransposeA(NgDialectObj.m_builder.getBoolAttr(matmulNode->get_transpose_a()));
    matmulOp.setTransposeB(NgDialectObj.m_builder.getBoolAttr(matmulNode->get_transpose_b()));
    return op;
}