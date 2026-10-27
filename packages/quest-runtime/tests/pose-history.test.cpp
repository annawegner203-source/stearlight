#include "../app/src/main/cpp/pose_history.h"
#include <cassert>

static quest::PoseSample sample(uint32_t sequence){quest::PoseSample s;s.sequence=sequence;s.position[0][0]=static_cast<float>(sequence);return s;}

int main(){
 quest::PoseHistory history;
 assert(!history.find(2,0)&&history.behind(2)==-1);
 for(uint32_t sequence=2;sequence<=20;sequence+=2)history.record(sample(sequence));
 assert(history.find(10,0)->sequence==10);
 assert(history.find(10,-1)->sequence==8&&history.find(10,2)->sequence==14);
 assert(history.find(10,-100)->sequence==2&&history.find(10,100)->sequence==20);
 assert(history.behind(20)==0&&history.behind(14)==3&&history.behind(11)==-1);
 // Session-state changes also advance the shared sequence: gaps are fine.
 history.record(sample(26));assert(history.find(20,1)->sequence==26&&history.behind(20)==1);
 // Only the newest 64 samples are retained.
 for(uint32_t sequence=28;sequence<28+2*200;sequence+=2)history.record(sample(sequence));
 assert(!history.find(26,0));
 uint32_t newest=28+2*199,oldest=newest-2*63;
 assert(history.find(newest,-1000)->sequence==oldest&&history.behind(oldest)==63);
 return 0;
}
