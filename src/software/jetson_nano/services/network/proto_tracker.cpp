#include "software/jetson_nano/services/network/proto_tracker.h"

ProtoTracker::ProtoTracker(const std::string& type) : ProtoCache(type) {}

void ProtoTracker::send(uint64_t seq_num)
{
    ProtoCache::send(seq_num);
    if (last_valid) {
        proto_loss_rate = calculate_proto_loss_rate(seq_num);
    }
}

float ProtoTracker::getLossRate()
{
    return proto_loss_rate;
}

float ProtoTracker::calculate_proto_loss_rate(uint64_t seq_num)
{
    // seq_num + 1 is to account for the sequence numbers starting from 0
    uint64_t expected_proto_count =
        std::min(seq_num + 1, static_cast<uint64_t>(RECENT_PROTO_LOSS_PERIOD));
    uint64_t lost_proto_count = expected_proto_count - recent_proto_seq_nums.size();
    float proto_loss_rate =
        static_cast<float>(lost_proto_count) / static_cast<float>(expected_proto_count);

    return proto_loss_rate;
}
