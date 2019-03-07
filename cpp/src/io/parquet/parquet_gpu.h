/*
* Copyright (c) 2018, NVIDIA CORPORATION.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef __IO_PARQUET_GPU_H__
#define __IO_PARQUET_GPU_H__

#include <cstdint>
#include "parquet_common.h"

namespace parquet { namespace gpu {

enum {
    PAGEINFO_FLAGS_DICTIONARY = 0x01,   // Indicates a dictionary page
};


// Data type to describe a string
struct nvstrdesc_s
{
    const char *ptr;
    size_t count;
};

struct PageInfo
{
    uint8_t *compressed_page_data;      // ptr to compressed page data (before decompression) or uncompressed data (after decompression)
    int32_t compressed_page_size;       // compressed data size in bytes
    int32_t uncompressed_page_size;     // uncompressed data size in bytes
    int32_t num_values;                 // Number of values in this data page or dictionary
    int32_t chunk_row;                  // starting row of this page relative to the start of the chunk
    int32_t num_rows;                   // number of rows in this page
    int32_t chunk_idx;                  // column chunk this page belongs to
    uint8_t flags;                      // PAGEINFO_FLAGS_XXX
    uint8_t encoding;                   // Encoding for data or dictionary page
    uint8_t definition_level_encoding;  // Encoding used for definition levels (data page)
    uint8_t repetition_level_encoding;  // Encoding used for repetition levels (data page)
    int32_t valid_count;                // Count of valid (non-null) values in this page (negative values indicate data error)
};

struct ColumnChunkDesc
{
    uint8_t *compressed_data;   // pointer to compressed column chunk data
    size_t compressed_size;     // total compressed data size for this chunk
    size_t num_values;          // total number of values in this column
    size_t start_row;           // starting row of this chunk
    uint32_t num_rows;          // number of rows in this chunk
    int16_t max_def_level;      // max definition level
    int16_t max_rep_level;      // max repetition level
    uint16_t data_type;         // basic column data type, ((type_length << 3) | parquet::Type)
    uint8_t def_level_bits;     // bits to encode max definition level
    uint8_t rep_level_bits;     // bits to encode max repetition level
    int32_t num_data_pages;     // number of data pages
    int32_t num_dict_pages;     // number of dictionary pages
    int32_t max_num_pages;      // size of page_info array
    PageInfo *page_info;        // output page info for up to num_dict_pages + num_data_pages (dictionary pages first)
    nvstrdesc_s *str_dict_index; // index for string dictionary
    uint32_t *valid_map_base;   // base pointer of valid bit map for this column
    void *column_data_base;     // base pointer of column data
    int8_t codec;               // compressed codec enum
};



// page_hdr.cu
cudaError_t DecodePageHeaders(ColumnChunkDesc *chunks, int32_t num_chunks, cudaStream_t stream = (cudaStream_t)0);
cudaError_t BuildStringDictionaryIndex(ColumnChunkDesc *chunks, int32_t num_chunks, cudaStream_t stream = (cudaStream_t)0);

// page_data.cu
cudaError_t DecodePageData(PageInfo *pages, int32_t num_pages, ColumnChunkDesc *chunks, int32_t num_chunks, size_t num_rows, size_t min_row = 0, cudaStream_t stream = (cudaStream_t)0);


};}; // parquet::gpu namespace


#endif // __IO_PARQUET_GPU_H__


