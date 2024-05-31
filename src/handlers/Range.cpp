#include "handlers.hpp"

#include "../character.hpp"
#include "../dialog.hpp"
#include "../eodata.hpp"
#include "../eoplus.hpp"
#include "../npc.hpp"
#include "../packet.hpp"
#include "../quest.hpp"
#include "../world.hpp"

#include "../util.hpp"

#include <cstddef>
#include <deque>
#include <memory>
#include <utility>

namespace Handlers
{

static void Range(Character *character, const std::vector<unsigned short>& player_ids, const std::vector<unsigned char>& npc_ids)
{
    auto in_range_characters = character->map->CharactersInRange(character->x, character->y, static_cast<int>(character->world->config["SeeDistance"]) + 1);
    auto in_range_npcs = character->map->NPCsInRange(character->x, character->y, static_cast<int>(character->world->config["SeeDistance"]) + 1);

    std::vector<Character*> players;
    for (const auto& id : player_ids)
    {
        auto c = std::find_if(
            in_range_characters.begin(),
            in_range_characters.end(),
            [id](Character * search_char) -> bool
            {
                return search_char->id == id;
            });
        if (c != in_range_characters.end())
        {
            players.push_back(*c);
        }
    }

    std::vector<NPC*> npcs;
    for (const auto& id : npc_ids)
    {
        auto n = std::find_if(
            in_range_npcs.begin(),
            in_range_npcs.end(),
            [id](NPC* search_npc) -> bool
            {
                return search_npc->index == id;
            });
        if (n != in_range_npcs.end())
        {
            npcs.push_back(*n);
        }
    }

    PacketBuilder reply(PACKET_RANGE, PACKET_REPLY, player_ids.size() * 42 + npc_ids.size() * 6 + 4);
    reply.AddChar(players.size());
    reply.AddByte(255);

    for (size_t i = 0; i < players.size(); i++)
    {
        reply.AddBreakString(players[i]->SourceName());
        reply.AddShort(players[i]->id);
        reply.AddShort(players[i]->mapid);
        reply.AddShort(players[i]->x);
        reply.AddShort(players[i]->y);
        reply.AddChar(players[i]->direction);
        reply.AddChar(players[i]->clas);
        reply.AddString(players[i]->PaddedGuildTag());
        reply.AddChar(players[i]->level);
        reply.AddChar(players[i]->gender);
        reply.AddChar(players[i]->hairstyle);
        reply.AddChar(players[i]->haircolor);
        reply.AddChar(players[i]->race);

        reply.AddShort(players[i]->maxhp);
        reply.AddShort(players[i]->hp);
        reply.AddShort(players[i]->maxtp);
        reply.AddShort(players[i]->tp);

        players[i]->AddPaperdollData(reply, "B000A0HSW");

        reply.AddChar(players[i]->sitting);
        reply.AddChar(players[i]->hidden > 0 ? 1 : 0);

        reply.AddByte(255);
    }

    for (size_t i = 0; i < npcs.size(); i++)
    {
        reply.AddChar(npcs[i]->index);
        reply.AddShort(npcs[i]->id);
        reply.AddChar(npcs[i]->x);
        reply.AddChar(npcs[i]->y);
        reply.AddChar(npcs[i]->direction);
    }

    reply.AddByte(255);

    character->Send(reply);
}

void Range_Request(Character *character, PacketReader &reader)
{
    std::vector<unsigned short> player_ids;
    std::vector<unsigned char> npc_ids;

    auto player_id_data = reader.GetBreakString();
    for (size_t i = 0; i < player_id_data.size(); i+=2)
    {
        player_ids.push_back(PacketProcessor::Number(player_id_data[i], player_id_data[i]+1));
    }

    while (reader.Remaining() > 0)
    {
        npc_ids.push_back(reader.GetChar());
    }

    Range(character, player_ids, npc_ids);
}

void PlayerRange_Request(Character *character, PacketReader &reader)
{
    std::vector<unsigned short> player_ids;
    while (reader.Remaining() > 0)
    {
        player_ids.push_back(reader.GetShort());
    }

    Range(character, player_ids, std::vector<unsigned char>());
}

void NPCRange_Request(Character *character, PacketReader &reader)
{
    std::vector<unsigned char> npc_ids;
    while (reader.Remaining() > 0)
    {
        npc_ids.push_back(reader.GetChar());
    }

    Range(character, std::vector<unsigned short>(), npc_ids);
}

PACKET_HANDLER_REGISTER(PACKET_RANGE)
	Register(PACKET_REQUEST, Range_Request, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_RANGE)

PACKET_HANDLER_REGISTER(PACKET_PLAYER_RANGE)
	Register(PACKET_REQUEST, PlayerRange_Request, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_PLAYER_RANGE)

PACKET_HANDLER_REGISTER(PACKET_NPC_RANGE)
	Register(PACKET_REQUEST, NPCRange_Request, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_NPC_RANGE)
}
