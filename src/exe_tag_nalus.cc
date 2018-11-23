#include "chef_filepath_op.hpp"
#include "chef_stuff_op.hpp"
#include <assert.h>
#include <iostream>
#include <sstream>
#include <map>

static std::map<int, std::string> nalu_unit_type_mapping = {
  {1, "SLICE"},
  {5, "IDR"},
  {6, "SEI"},
  {7, "SPS"},
  {8, "PPS"},
  {9, "AUD"}
};

static constexpr int FIELD_LENGTH_FLV_HEADER   = 9;
static constexpr int FIELD_LENGTH_PRE_TAG_SIZE = 4;

static constexpr int TAG_TYPE_AUDIO       = 8;
static constexpr int TAG_TYPE_VIDEO       = 9;
static constexpr int TAG_TYPE_SCRIPT_DATA = 18;

static constexpr int PACKET_TYPE_SEQ_HEADER = 0;
static constexpr int PACKET_TYPE_NALU       = 1;

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <flv file>" << std::endl;
    return -1;
  }

  char *filename = argv[1];
  int data_size;

  std::string whole_content = chef::filepath_op::read_file(filename);

  const char *p = whole_content.c_str();
  assert(chef::stuff_op::read_be_int((const uint8_t *)p+5, 4) == FIELD_LENGTH_FLV_HEADER);

  // skip FLV Header and First Tag Size
  p += FIELD_LENGTH_FLV_HEADER + FIELD_LENGTH_PRE_TAG_SIZE;

  int video_tag_count = 0;

  for (; ; ) {
    int tag_type = *p++;

    // 测试时发现wget的flv文件后面会有一些 00 00 00 的结尾，所以数据非法直接break
    if (tag_type != TAG_TYPE_AUDIO && tag_type != TAG_TYPE_VIDEO && tag_type != TAG_TYPE_SCRIPT_DATA) { break; }

    data_size = chef::stuff_op::read_be_int((const uint8_t *)p, 3);

    // skip Metadata Tag or Audio Tag
    if (tag_type != TAG_TYPE_VIDEO) { p += 10 + data_size + FIELD_LENGTH_PRE_TAG_SIZE; continue; }

    // skip Tag Header(data size + timestamp + stream id)
    p += 10;

    int frame_type = *p++;
    int packet_type = *p++;

    std::cout << "Video Tag" << video_tag_count++ << ": ";

    // skip sequence header
    if (packet_type == 0) {
      std::cout << "sequence header\n";
      p += data_size + 2;
      continue;
    }

    assert(packet_type == PACKET_TYPE_NALU);

    int composition_time = chef::stuff_op::read_be_int((const uint8_t *)p, 3);
    assert(composition_time == 0);
    p += 3;

    for (int i = 5; i != data_size; ) {
      int nalu_len = chef::stuff_op::read_be_int((const uint8_t *)p, 4);
      p += 4;
      int nalu_unit_type = *p & 0x1f;
      assert(!nalu_unit_type_mapping[nalu_unit_type].empty());
      std::cout << " " << nalu_unit_type_mapping[nalu_unit_type];
      p += nalu_len;
      i += 4 + nalu_len;
    }

    std::cout << "\n";

    // skip pre tag size
    p += FIELD_LENGTH_PRE_TAG_SIZE;

    if (p - whole_content.c_str() == whole_content.length()) { break; }

  }

  return 0;
}
