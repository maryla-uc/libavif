// Copyright 2022 Google LLC
// SPDX-License-Identifier: BSD-2-Clause

#include "avif_fuzztest_helpers.h"

#include <algorithm>
#include <cassert>

#include "avif/avif.h"

namespace libavif {
namespace testutil {
namespace {

//------------------------------------------------------------------------------

AvifImagePtr CreateAvifImage(size_t width, size_t height, int depth,
                             avifPixelFormat pixel_format, bool has_alpha,
                             const uint8_t* samples) {
  AvifImagePtr image(avifImageCreate(width, height, depth, pixel_format),
                     avifImageDestroy);
  if (image.get() == nullptr) {
    return image;
  }
  avifImageAllocatePlanes(image.get(),
                          has_alpha ? AVIF_PLANES_ALL : AVIF_PLANES_YUV);

  for (avifChannelIndex c :
       {AVIF_CHAN_Y, AVIF_CHAN_U, AVIF_CHAN_V, AVIF_CHAN_A}) {
    const size_t plane_height = avifImagePlaneHeight(image.get(), c);
    uint8_t* row = avifImagePlane(image.get(), c);
    const uint32_t row_bytes = avifImagePlaneRowBytes(image.get(), c);
    assert(row_bytes == avifImagePlaneWidth(image.get(), c) *
                            (avifImageUsesU16(image.get()) ? 2 : 1));
    for (size_t y = 0; y < plane_height; ++y) {
      std::copy(samples, samples + row_bytes, row);
      row += row_bytes;
      samples += row_bytes;
    }
  }
  return image;
}

}  // namespace

avifImage* CreateAvifImage8bRawPtr(size_t width, size_t height,
                                   avifPixelFormat pixel_format, bool has_alpha,
                                   const std::vector<uint8_t>& samples) {
  return CreateAvifImage(width, height, 8, pixel_format, has_alpha,
                         samples.data())
      .release();
}

avifImage* CreateAvifImage16bRawPtr(size_t width, size_t height, int depth,
                                    avifPixelFormat pixel_format,
                                    bool has_alpha,
                                    const std::vector<uint16_t>& samples) {
  return CreateAvifImage(width, height, depth, pixel_format, has_alpha,
                         reinterpret_cast<const uint8_t*>(samples.data()))
      .release();
}

AvifEncoderPtr CreateAvifEncoder(avifCodecChoice codec_choice, int max_threads,
                                 int min_quantizer, int max_quantizer,
                                 int min_quantizer_alpha,
                                 int max_quantizer_alpha, int tile_rows_log2,
                                 int tile_cols_log2, int speed) {
  AvifEncoderPtr encoder(avifEncoderCreate(), avifEncoderDestroy);
  if (encoder.get() == nullptr) {
    return encoder;
  }
  encoder->codecChoice = codec_choice;
  encoder->maxThreads = max_threads;
  // minQuantizer must be at most maxQuantizer.
  encoder->minQuantizer = std::min(min_quantizer, max_quantizer);
  encoder->maxQuantizer = std::max(min_quantizer, max_quantizer);
  encoder->minQuantizerAlpha =
      std::min(min_quantizer_alpha, max_quantizer_alpha);
  encoder->maxQuantizerAlpha =
      std::max(min_quantizer_alpha, max_quantizer_alpha);
  encoder->tileRowsLog2 = tile_rows_log2;
  encoder->tileColsLog2 = tile_cols_log2;
  encoder->speed = speed;
  return encoder;
}

AvifDecoderPtr CreateAvifDecoder(avifCodecChoice codec_choice, int max_threads,
                                 avifDecoderSource requested_source,
                                 bool allow_progressive, bool allow_incremental,
                                 bool ignore_exif, bool ignore_xmp,
                                 uint32_t image_size_limit,
                                 uint32_t image_dimension_limit,
                                 uint32_t image_count_limit,
                                 avifStrictFlags strict_flags) {
  AvifDecoderPtr decoder(avifDecoderCreate(), avifDecoderDestroy);
  if (decoder.get() == nullptr) {
    return decoder;
  }
  decoder->codecChoice = codec_choice;
  decoder->maxThreads = max_threads;
  decoder->requestedSource = requested_source;
  decoder->allowProgressive = allow_progressive;
  decoder->allowIncremental = allow_incremental;
  decoder->ignoreExif = ignore_exif;
  decoder->ignoreXMP = ignore_xmp;
  decoder->imageSizeLimit = image_size_limit;
  decoder->imageDimensionLimit = image_dimension_limit;
  decoder->imageCountLimit = image_count_limit;
  decoder->strictFlags = strict_flags;
  return decoder;
}

AvifImagePtr AvifImageToUniquePtr(avifImage* image) {
  return AvifImagePtr(image, avifImageDestroy);
}

//------------------------------------------------------------------------------

size_t GetNumSamples(size_t width, size_t height, avifPixelFormat pixel_format,
                     bool has_alpha) {
  const size_t num_luma_samples = width * height;

  avifPixelFormatInfo pixel_format_info;
  avifGetPixelFormatInfo(pixel_format, &pixel_format_info);
  size_t num_chroma_samples = 0;
  if (!pixel_format_info.monochrome) {
    num_chroma_samples = 2 *
                         ((width + pixel_format_info.chromaShiftX) >>
                          pixel_format_info.chromaShiftX) *
                         ((height + pixel_format_info.chromaShiftY) >>
                          pixel_format_info.chromaShiftY);
  }

  size_t num_alpha_samples = 0;
  if (has_alpha) {
    num_alpha_samples = width * height;
  }

  return num_luma_samples + num_chroma_samples + num_alpha_samples;
}

//------------------------------------------------------------------------------

std::vector<uint8_t> GetWhiteSinglePixelAvif() {
  return {
      0x0,  0x0,  0x0,  0x20, 0x66, 0x74, 0x79, 0x70, 0x61, 0x76, 0x69, 0x66,
      0x0,  0x0,  0x0,  0x0,  0x61, 0x76, 0x69, 0x66, 0x6d, 0x69, 0x66, 0x31,
      0x6d, 0x69, 0x61, 0x66, 0x4d, 0x41, 0x31, 0x41, 0x0,  0x0,  0x0,  0xf2,
      0x6d, 0x65, 0x74, 0x61, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x28,
      0x68, 0x64, 0x6c, 0x72, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
      0x70, 0x69, 0x63, 0x74, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
      0x0,  0x0,  0x0,  0x0,  0x6c, 0x69, 0x62, 0x61, 0x76, 0x69, 0x66, 0x0,
      0x0,  0x0,  0x0,  0xe,  0x70, 0x69, 0x74, 0x6d, 0x0,  0x0,  0x0,  0x0,
      0x0,  0x1,  0x0,  0x0,  0x0,  0x1e, 0x69, 0x6c, 0x6f, 0x63, 0x0,  0x0,
      0x0,  0x0,  0x44, 0x0,  0x0,  0x1,  0x0,  0x1,  0x0,  0x0,  0x0,  0x1,
      0x0,  0x0,  0x1,  0x1a, 0x0,  0x0,  0x0,  0x17, 0x0,  0x0,  0x0,  0x28,
      0x69, 0x69, 0x6e, 0x66, 0x0,  0x0,  0x0,  0x0,  0x0,  0x1,  0x0,  0x0,
      0x0,  0x1a, 0x69, 0x6e, 0x66, 0x65, 0x2,  0x0,  0x0,  0x0,  0x0,  0x1,
      0x0,  0x0,  0x61, 0x76, 0x30, 0x31, 0x43, 0x6f, 0x6c, 0x6f, 0x72, 0x0,
      0x0,  0x0,  0x0,  0x6a, 0x69, 0x70, 0x72, 0x70, 0x0,  0x0,  0x0,  0x4b,
      0x69, 0x70, 0x63, 0x6f, 0x0,  0x0,  0x0,  0x14, 0x69, 0x73, 0x70, 0x65,
      0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1,  0x0,  0x0,  0x0,  0x1,
      0x0,  0x0,  0x0,  0x10, 0x70, 0x69, 0x78, 0x69, 0x0,  0x0,  0x0,  0x0,
      0x3,  0x8,  0x8,  0x8,  0x0,  0x0,  0x0,  0xc,  0x61, 0x76, 0x31, 0x43,
      0x81, 0x20, 0x0,  0x0,  0x0,  0x0,  0x0,  0x13, 0x63, 0x6f, 0x6c, 0x72,
      0x6e, 0x63, 0x6c, 0x78, 0x0,  0x1,  0x0,  0xd,  0x0,  0x6,  0x80, 0x0,
      0x0,  0x0,  0x17, 0x69, 0x70, 0x6d, 0x61, 0x0,  0x0,  0x0,  0x0,  0x0,
      0x0,  0x0,  0x1,  0x0,  0x1,  0x4,  0x1,  0x2,  0x83, 0x4,  0x0,  0x0,
      0x0,  0x1f, 0x6d, 0x64, 0x61, 0x74, 0x12, 0x0,  0xa,  0x7,  0x38, 0x0,
      0x6,  0x10, 0x10, 0xd0, 0x69, 0x32, 0xa,  0x1f, 0xf0, 0x3f, 0xff, 0xff,
      0xc4, 0x0,  0x0,  0xaf, 0x70};
}

//------------------------------------------------------------------------------

template <typename SampleType>
bool IsOpaque(const avifImage* image) {
  if (!image->alphaPlane) {
    return true;
  }

  const SampleType opaque_alpha = (1 << image->depth) - 1;
  for (uint32_t j = 0; j < image->height; ++j) {
    const SampleType* const row = reinterpret_cast<SampleType*>(
        image->alphaPlane + j * image->alphaRowBytes);
    for (uint32_t i = 0; i < image->width; ++i) {
      if (row[i] != opaque_alpha) {
        assert(row[i] < opaque_alpha);
        return false;
      }
    }
  }
  return true;
}

bool IsOpaque(const avifImage* image) {
  return avifImageUsesU16(image) ? IsOpaque<uint16_t>(image)
                                 : IsOpaque<uint8_t>(image);
}

//------------------------------------------------------------------------------

}  // namespace testutil
}  // namespace libavif
