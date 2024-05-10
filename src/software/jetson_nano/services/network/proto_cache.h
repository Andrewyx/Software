#include <queue>
#include <string>

#include "software/logger/logger.h"

class ProtoCache
{
public:
    /**
     * Caches the sequence numbers of recently received received protos
     *
     * @param type The name of the type of protos being tracked
     */
    ProtoCache(const std::string& type);

    /**
     * When a new sequence number is sent, the ProtoCache updates the protos in the cache
     *
     * @param seq_num The sequence number of the newly received proto
     */
    virtual void send(uint64_t seq_num);

    /**
     * @return a boolean indicating whether the last received proto was valid
     */
    bool isLastValid();

protected:
    // Methods
    /**
     * Removes sequence numbers of protos that are no longer relevant
     *
     * @param seq_num The sequence number of a given proto
     */
    virtual void removeOutdatedProtos(uint64_t seq_num);

    // Constants
    static constexpr uint8_t RECENT_PROTO_LOSS_PERIOD = 100;

    // Variables
    std::string proto_type;
    bool last_valid       = false;

    // Queue of the sequence numbers of received protos in the past
    std::queue<uint64_t> recent_proto_seq_nums;
};