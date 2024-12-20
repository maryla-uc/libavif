// Copyright 2022 Google LLC
// SPDX-License-Identifier: BSD-2-Clause
// Compare non-incremental and incremental decode results of an arbitrary byte
// sequence.

#include <algorithm>
#include <cstdint>
#include <string>

#include "avif/avif.h"
#include "avif_fuzztest_helpers.h"
#include "avifincrtest_helpers.h"
#include "fuzztest/fuzztest.h"
#include "gtest/gtest.h"

using ::fuzztest::Arbitrary;

namespace avif {
namespace testutil {
namespace {

//------------------------------------------------------------------------------

void DecodeTruncated(const std::string& arbitrary_bytes) {
  ASSERT_FALSE(GetSeedDataDirs().empty());  // Make sure seeds are available.

  ImagePtr reference(avifImageCreateEmpty());
  ASSERT_NE(reference.get(), nullptr);

  const uint8_t* data =
      reinterpret_cast<const uint8_t*>(arbitrary_bytes.data());
  avifIO* const io = avifIOCreateMemoryReader(data, arbitrary_bytes.size());
  if (io == nullptr) return;

  DecoderPtr decoder(avifDecoderCreate());
  ASSERT_NE(decoder.get(), nullptr);
  avifDecoderSetIO(decoder.get(), io);
  // OSS-Fuzz limits the allocated memory to 2560 MB.
  constexpr uint32_t kMaxMem = 2560u * 1024 * 1024;
  // Consider at most four planes of 16-bit samples.
  constexpr uint32_t kMaxImageSize =
      kMaxMem / (AVIF_PLANE_COUNT_YUV + 1) / sizeof(uint16_t);
  // Reduce the limit further to include pixel buffer copies and other memory
  // allocations.
  constexpr uint32_t kImageSizeLimit = kMaxImageSize / 4;
  // avifDecoderParse returns AVIF_RESULT_NOT_IMPLEMENTED if kImageSizeLimit is
  // bigger than AVIF_DEFAULT_IMAGE_SIZE_LIMIT.
  static_assert(kImageSizeLimit <= AVIF_DEFAULT_IMAGE_SIZE_LIMIT,
                "Too big an image size limit");
  decoder->imageSizeLimit = kImageSizeLimit;

  avifResult result = avifDecoderRead(decoder.get(), reference.get());
  if (result == AVIF_RESULT_OK) {
    for (size_t truncated_size = 1; truncated_size < arbitrary_bytes.size();
         ++truncated_size) {
      decoder.reset(avifDecoderCreate());
      decoder->imageSizeLimit = kImageSizeLimit;
      avifIO* const io = avifIOCreateMemoryReader(data, truncated_size);
      if (io == nullptr) return;
      avifDecoderSetIO(decoder.get(), io);
      result = avifDecoderRead(decoder.get(), reference.get());
      ASSERT_TRUE(result == AVIF_RESULT_OK ||
                  result == AVIF_RESULT_TRUNCATED_DATA)
          << "with " << arbitrary_bytes.size() << " byte stream truncated to "
          << truncated_size << " bytes, error: " << avifResultToString(result)
          << " " << decoder->diag.error;
    }
  }
}

FUZZ_TEST(DecodeAvifFuzzTest, DecodeTruncated)
    .WithDomains(ArbitraryImageWithSeeds({AVIF_APP_FILE_FORMAT_AVIF}));

//------------------------------------------------------------------------------

}  // namespace
}  // namespace testutil
}  // namespace avif
