#include <queue>
#include <string>

#include "software/jetson_nano/services/network/proto_cache.h"
#include "software/logger/logger.h"

class ProtoTracker: public ProtoCache
{
   public:
    /**
     * Tracks recent proto loss given the sequence numbers of received protos
     *
     * @param type The name of the type of protos being tracked
     */
    ProtoTracker(const std::string& type);

    /**
     * When a new sequence number is sent, the ProtoTracker updates the proto tracking
     *
     * @param seq_num The sequence number of the newly received proto
     */
    void send(uint64_t seq_num) override;

    /**
     * @return a float equal to the proto loss rate
     */
    float getLossRate();

   private:
    /**
     * Private function for calculating the proto loss rate
     *
     * @param seq_num The sequence number of the newly received protobuf
     * @return a float equal to the proto loss rate
     */
    float calculate_proto_loss_rate(uint64_t seq_num);

    // Variables
    float proto_loss_rate = 0;
};
