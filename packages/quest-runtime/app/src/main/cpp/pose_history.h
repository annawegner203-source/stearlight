#pragma once
// Recent published eye poses by tracking sequence. Linux tags each image with
// the sample it read at swapchain acquire; SteamVR may have rendered with a
// neighbouring sample. A small offset re-tags the image with that neighbour.
#include <cstdint>
namespace quest {
struct PoseSample { uint32_t sequence=0; uint64_t predicted_ns=0; float position[2][3]{},orientation[2][4]{}; };
class PoseHistory {
 static constexpr uint32_t size=64;
 PoseSample samples[size];uint32_t count=0;
 int64_t indexOf(uint32_t sequence)const{
  uint32_t available=count<size?count:size;
  for(uint32_t back=0;back<available;back++){uint32_t index=count-1-back;if(samples[index%size].sequence==sequence)return index;}
  return -1;
 }
public:
 void record(const PoseSample& sample){samples[count%size]=sample;++count;}
 // The sample `offset` publications after the tagged one; negative is older.
 // Out-of-range offsets clamp to the oldest or newest retained sample.
 const PoseSample* find(uint32_t sequence,int offset)const{
  int64_t index=indexOf(sequence);if(index<0)return nullptr;
  int64_t oldest=count>size?count-size:0,target=index+offset;
  if(target<oldest)target=oldest;
  if(target>=count)target=count-1;
  return &samples[target%size];
 }
 // Publications between the tagged sample and the newest one, or -1.
 int behind(uint32_t sequence)const{int64_t index=indexOf(sequence);return index<0?-1:static_cast<int>(count-1-index);}
};
}
