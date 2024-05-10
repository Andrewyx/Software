#include "software/jetson_nano/services/network/proto_cache.h"

ProtoCache::ProtoCache(const std::string& type) : proto_type(type) {}

void ProtoCache::send(uint64_t seq_num)
{
    // If the proto seems very out of date, then this is likely due to an AI reset. Clear
    // the queue
    if (!recent_proto_seq_nums.empty() &&
        seq_num + RECENT_PROTO_LOSS_PERIOD <= recent_proto_seq_nums.back())
    {
        recent_proto_seq_nums = std::queue<uint64_t>();
        LOG(WARNING) << "Old " << proto_type
                     << " received. Resetting sequence number tracking.";
    }
    else if (!recent_proto_seq_nums.empty() && seq_num <= recent_proto_seq_nums.back())
    {
        // If the proto is older than the last received proto, then ignore it
        last_valid = false;
        return;
    }

    recent_proto_seq_nums.push(seq_num);
    // Pop sequence numbers of protos that are no longer recent
    while (seq_num - recent_proto_seq_nums.front() >= RECENT_PROTO_LOSS_PERIOD)
    {
        recent_proto_seq_nums.pop();
    }

    last_valid = true;
}

bool ProtoCache::isLastValid()
{
    return last_valid;
}
