#include "software/ai/hl/stp/play/enemy_free_kick/enemy_free_kick_play_fsm.h"

#include "proto/parameters.pb.h"
#include "shared/constants.h"
#include "software/ai/evaluation/enemy_threat.h"
#include "software/ai/hl/stp/tactic/crease_defender/crease_defender_tactic.h"
#include "software/ai/hl/stp/tactic/move/move_tactic.h"
#include "software/ai/hl/stp/tactic/shadow_enemy/shadow_enemy_tactic.h"
#include "software/util/generic_factory/generic_factory.h"
#include "software/world/game_state.h"

EnemyFreeKickPlayFSM::EnemyFreeKickPlayFSM(TbotsProto::AiConfig config)
: ai_config(ai_config),
  defense_play(std::make_shared<DefensePlay>(ai_config))
{
}

void EnemyFreeKickPlayFSM::setupEnemyKickerStrategy(const Update& event)
{
    setTactics(event, 1, (event.common.num_tactics - 1));
}

void EnemyFreeKickPlayFSM::setTactics(const Update& event, int num_shadow_robots,
                                              int num_defenders)
{
    PriorityTacticVector tactics_to_return;

//    if (num_shadow_robots > 0)
//    {
//        shoot_or_pass_play->updateTactics(PlayUpdate(
//                event.common.world_ptr, num_shadow_robots,
//                [&tactics_to_return](PriorityTacticVector new_tactics) {
//                    for (const auto& tactic_vector : new_tactics)
//                    {
//                        tactics_to_return.push_back(tactic_vector);
//                    }
//                },
//                event.common.inter_play_communication,
//                event.common.set_inter_play_communication_fun));
//    }
//TODO: SILLY CODE FOR SHADOW TACTIC

    defense_play->updateControlParams(TbotsProto::MaxAllowedSpeedMode::PHYSICAL_LIMIT);

    if (num_defenders > 0)
    {
        defense_play->updateTactics(PlayUpdate(
                event.common.world_ptr, num_defenders,
                [&tactics_to_return](PriorityTacticVector new_tactics) {
                    for (const auto& tactic_vector : new_tactics)
                    {
                        tactics_to_return.push_back(tactic_vector);
                    }
                },
                event.common.inter_play_communication,
                event.common.set_inter_play_communication_fun));
    }

    event.common.set_tactics(tactics_to_return);
}