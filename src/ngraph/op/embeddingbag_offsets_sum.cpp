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

#include "ngraph/op/embeddingbag_offsets_sum.hpp"
#include "ngraph/op/constant.hpp"

using namespace std;
using namespace ngraph;

constexpr NodeTypeInfo op::v3::EmbeddingBagOffsetsSum::type_info;

op::v3::EmbeddingBagOffsetsSum::EmbeddingBagOffsetsSum(const Output<Node>& emb_table,
                                                       const Output<Node>& indices,
                                                       const Output<Node>& offsets,
                                                       const Output<Node>& per_sample_weights,
                                                       const Output<Node>& default_index)
    : Op({emb_table, indices, offsets, per_sample_weights, default_index})
{
    constructor_validate_and_infer_types();
}

op::v3::EmbeddingBagOffsetsSum::EmbeddingBagOffsetsSum(const Output<Node>& emb_table,
                                                       const Output<Node>& indices,
                                                       const Output<Node>& offsets,
                                                       const Output<Node>& per_sample_weights)
    : Op({emb_table, indices, offsets, per_sample_weights})
{
    constructor_validate_and_infer_types();
}

op::v3::EmbeddingBagOffsetsSum::EmbeddingBagOffsetsSum(const Output<Node>& emb_table,
                                                       const Output<Node>& indices,
                                                       const Output<Node>& offsets)
    : Op({emb_table, indices, offsets})
{
    constructor_validate_and_infer_types();
}

void op::v3::EmbeddingBagOffsetsSum::validate_and_infer_types()
{
    enum
    {
        EMB_TABLE,
        INDICES,
        OFFSETS,
        PER_SAMPLE_WEIGHTS,
        DEFAULT_INDEX
    };

    NODE_VALIDATION_CHECK(this,
                          get_input_element_type(OFFSETS) == element::i64 ||
                              get_input_element_type(OFFSETS) == element::i32,
                          "OFFSETS type must be i32 or i64");

    NODE_VALIDATION_CHECK(this,
                          get_input_element_type(INDICES) == element::i64 ||
                              get_input_element_type(INDICES) == element::i32,
                          "INDICES type must be i32 or i64");

    NODE_VALIDATION_CHECK(
        this,
        get_input_element_type(INDICES).compatible(get_input_element_type(OFFSETS)),
        "Offsets element type (",
        get_input_element_type(OFFSETS),
        ") must match indices element type (",
        get_input_element_type(INDICES),
        ")");

    NODE_VALIDATION_CHECK(this,
                          get_input_partial_shape(INDICES).is_dynamic() ||
                              get_input_partial_shape(INDICES).to_shape().size() == 1,
                          "INDICES must be 1D");

    NODE_VALIDATION_CHECK(this,
                          get_input_partial_shape(OFFSETS).is_dynamic() ||
                              get_input_partial_shape(OFFSETS).to_shape().size() == 1,
                          "OFFSETS must be 1D");

    if (get_input_size() >= 4)
    {
        NODE_VALIDATION_CHECK(this,
                              get_input_element_type(EMB_TABLE).compatible(
                                  get_input_element_type(PER_SAMPLE_WEIGHTS)),
                              "Per sample weight element type (",
                              get_input_element_type(PER_SAMPLE_WEIGHTS),
                              ") must match embedding table element type (",
                              get_input_element_type(EMB_TABLE),
                              ")");

        NODE_VALIDATION_CHECK(this,
                              get_input_partial_shape(PER_SAMPLE_WEIGHTS).is_dynamic() ||
                                  get_input_partial_shape(PER_SAMPLE_WEIGHTS).to_shape().size() ==
                                      1,
                              "PER_SAMPLE_WEIGHTS must be 1D");

        NODE_VALIDATION_CHECK(this,
                              get_input_partial_shape(INDICES).compatible(
                                  get_input_partial_shape(PER_SAMPLE_WEIGHTS)),
                              "INDICES and PER_SAMPLE_WEIGHTS shape must be same");
    }

    if (get_input_size() == 5)
    {
        NODE_VALIDATION_CHECK(this,
                              get_input_element_type(DEFAULT_INDEX) == element::i64 ||
                                  get_input_element_type(DEFAULT_INDEX) == element::i32,
                              "DEFAULT_INDEX type must be i32 or i64");

        NODE_VALIDATION_CHECK(
            this,
            get_input_element_type(INDICES).compatible(get_input_element_type(DEFAULT_INDEX)),
            "Default_index element type (",
            get_input_element_type(DEFAULT_INDEX),
            ") must match indices element type (",
            get_input_element_type(INDICES),
            ")");

        NODE_VALIDATION_CHECK(this,
                              get_input_partial_shape(DEFAULT_INDEX).compatible(PartialShape{}),
                              "DEFAULT_INDEX must be a scalar");
    }

    element::Type result_et = get_input_element_type(EMB_TABLE);

    const PartialShape& emb_table_shape = get_input_partial_shape(EMB_TABLE);
    const PartialShape& offsets_shape = get_input_partial_shape(OFFSETS);

    PartialShape result_shape;
    if (emb_table_shape.rank().is_static())
    {
        std::vector<Dimension> result_dims(emb_table_shape.rank().get_length());
        result_dims[0] = offsets_shape.rank().is_static() ? offsets_shape[0] : Dimension::dynamic();
        for (size_t i = 1; i < emb_table_shape.rank().get_length(); i++)
        {
            result_dims[i] = emb_table_shape[i];
        }

        result_shape = PartialShape(result_dims);
    }
    else
    {
        result_shape = PartialShape::dynamic();
    }

    set_output_type(0, result_et, result_shape);
}

bool op::EmbeddingBagOffsetsSum::visit_attributes(AttributeVisitor& visitor)
{
    return true;
}

shared_ptr<Node>
    op::v3::EmbeddingBagOffsetsSum::clone_with_new_inputs(const OutputVector& new_args) const
{
    check_new_args_count(this, new_args);
    if (new_args.size() == 3)
    {
        return make_shared<EmbeddingBagOffsetsSum>(new_args.at(0), new_args.at(1), new_args.at(2));
    }
    else if (new_args.size() == 4)
    {
        return make_shared<EmbeddingBagOffsetsSum>(
            new_args.at(0), new_args.at(1), new_args.at(2), new_args.at(3));
    }
    else if (new_args.size() == 5)
    {
        return make_shared<EmbeddingBagOffsetsSum>(
            new_args.at(0), new_args.at(1), new_args.at(2), new_args.at(3), new_args.at(4));
    }
    else
    {
        throw ngraph_error("Incorrect number of arguments");
    }
}
