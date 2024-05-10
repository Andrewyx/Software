#include "software/jetson_nano/services/network/proto_cache.h"

class ProtoTrackerRTT: public ProtoCache
{
public:
    /**
     * Tracks recent sequence numbers of received primitive set protos for round-trip time (RTT) calculations
     *
     * @param type The name of the type of protos being tracked
     */
    ProtoTrackerRTT(const std::string& type);

    /**
     * When a new sequence number is sent, the ProtoTrackerRTT updates the protos in the cache
     *
     * @param seq_num The sequence number of the newly received proto
     */
    void send(uint64_t seq_num) override;


private:
    // Methods
    /**
     * Removes cached protos that are older than the given sequence number
     * @param seq_num The sequence number of a given proto
     */
    void removeOutdatedProtos(uint64_t seq_num) override;
};