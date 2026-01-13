#include "program_command.h"

#include <sstream>
#include <vector>

namespace avif {

void GridOptions::Init(argparse::ArgumentParser& argparse) {
  argparse.add_argument(arg_grid, "--grid")
      .help(
          "Encode a single-image grid AVIF with M cols & N rows, in the format "
          "'MxN'. The input image will be split into an MxN grid of cells, "
          "and the grid will adopt the color profile of the input image.")
      .default_value("1x1");
}

avifResult GridOptions::Parse() {
  std::vector<int> grid_dims;
  if (!ParseList(arg_grid.value(), 'x', 2, &grid_dims) || grid_dims[0] <= 0 ||
      grid_dims[1] <= 0) {
    std::cerr << "Invalid grid dimensions: " << arg_grid.value() << "\n";
    return AVIF_RESULT_INVALID_ARGUMENT;
  }
  grid_cols = grid_dims[0];
  grid_rows = grid_dims[1];
  return AVIF_RESULT_OK;
}

ProgramCommand::ProgramCommand(const std::string& name,
                               const std::string& short_description,
                               const std::string& long_description)
    : argparse_(argparse::ArgumentParser(
          "avifgainmaputil " + name,
          short_description + ".\n" + long_description)),
      name_(name),
      short_description_(short_description) {}

// Parses command line arguments. Should be called before Run().
avifResult ProgramCommand::ParseArgs(int argc, const char* const argv[]) {
  argparse_.parse_args(argc, argv);
  return PostParse();
}

avifResult ProgramCommand::PostParse() { return AVIF_RESULT_OK; }

// Prints this command's help on stdout.
void ProgramCommand::PrintUsage() { argparse_.print_help(); }

argparse::ConvertedValue<int> PixelFormatConverter::from_str(
    const std::string& str) {
  argparse::ConvertedValue<int> converted_value;

  if (str == "444") {
    converted_value.set_value(AVIF_PIXEL_FORMAT_YUV444);
  } else if (str == "422") {
    converted_value.set_value(AVIF_PIXEL_FORMAT_YUV422);
  } else if (str == "420") {
    converted_value.set_value(AVIF_PIXEL_FORMAT_YUV420);
  } else if (str == "400") {
    converted_value.set_value(AVIF_PIXEL_FORMAT_YUV400);
  } else {
    converted_value.set_error("Invalid argument value");
  }
  return converted_value;
}

std::vector<std::string> PixelFormatConverter::default_choices() {
  return {"444", "422", "420", "400"};
}

argparse::ConvertedValue<CicpValues> CicpConverter::from_str(
    const std::string& str) {
  argparse::ConvertedValue<CicpValues> converted_value;

  std::vector<uint32_t> cicp_values;
  if (!ParseList(str, '/', 3, &cicp_values)) {
    converted_value.set_error(
        "Invalid CICP values, expected format: P/T/M where each "
        "value is a positive integer, got: " +
        str);
  }

  CicpValues cicp = {};
  cicp.color_primaries = (avifColorPrimaries)cicp_values[0];
  cicp.transfer_characteristics = (avifTransferCharacteristics)cicp_values[1];
  cicp.matrix_coefficients = (avifMatrixCoefficients)cicp_values[2];
  converted_value.set_value(cicp);

  return converted_value;
}

std::vector<std::string> CicpConverter::default_choices() { return {}; }

}  // namespace avif
