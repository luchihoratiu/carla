#include "OpenDrive.h"
#include "parser/OpenDriveParser.h"

#include "../road/MapBuilder.h"

namespace carla {
namespace opendrive {

  #define UNUSED(x) (void)x

  struct lane_junction_t {
    int connection_road = -1;
    int from_lane = 0, to_lane = 0;
  };

  static void fnc_generate_roads_data(opendrive::types::OpenDriveData &openDriveRoad, std::map<int, opendrive::types::RoadInformation *> &out_roads) {
    for (size_t i = 0; i < openDriveRoad.roads.size(); ++i) {
      out_roads[openDriveRoad.roads[i].attributes.id] = &openDriveRoad.roads[i];
    }
  }

  static void fnc_generate_junctions_data(opendrive::types::OpenDriveData &openDriveRoad, std::map<int, std::map<int, std::vector<lane_junction_t>>> &out_data) {
    for (size_t i = 0; i < openDriveRoad.junctions.size(); ++i) {
      for (size_t j = 0; j < openDriveRoad.junctions[i].connections.size(); ++j) {
        lane_junction_t junctionData;
        int junctionID = openDriveRoad.junctions[i].attributes.id;

        int incommingRoad = openDriveRoad.junctions[i].connections[j].attributes.incoming_road;
        int connectingRoad = openDriveRoad.junctions[i].connections[j].attributes.connecting_road;

        junctionData.connection_road = connectingRoad;

        if (openDriveRoad.junctions[i].connections[j].links.size()) {
          junctionData.from_lane = openDriveRoad.junctions[i].connections[j].links[0].from;
          junctionData.to_lane = openDriveRoad.junctions[i].connections[j].links[0].to;
        }

        out_data[junctionID][incommingRoad].push_back(junctionData);
      }
    }
  }

  road::Map OpenDrive::Load(const std::string &file) {
    carla::opendrive::types::OpenDriveData open_drive_road;
    OpenDriveParser::Parse(file.c_str(), open_drive_road);

    if (open_drive_road.roads.empty()) {
      // TODO(Andrei): Log some type of warning
      return road::Map();
    }

    // Generate road and junction information
    using junction_data_t = std::map<int, std::map<int, std::vector<lane_junction_t>>>;
    using road_data_t = std::map<int, carla::opendrive::types::RoadInformation *>;

    carla::road::MapBuilder mapBuilder;
    junction_data_t junctionsData;
    road_data_t roadData;

    fnc_generate_roads_data(open_drive_road, roadData);
    fnc_generate_junctions_data(open_drive_road, junctionsData);

    // Transforma data for the MapBuilder
    for(road_data_t::iterator it = roadData.begin(); it != roadData.end(); ++it)
    {
      carla::road::RoadSegmentDefinition roadSegment;
      roadSegment.id = it->first;

      if(it->second->road_link.successor != nullptr)
      {
        if(it->second->road_link.successor->element_type == "junction")
        {
          std::vector<lane_junction_t> & options = junctionsData[it->second->road_link.successor->id][it->first];
          for(size_t i = 0; i < options.size(); ++i) roadSegment.successor_id.push_back(options[i].connection_road);
        }
        else
        {
          roadSegment.successor_id.push_back(it->second->road_link.successor->id);
        }
      }

      if(it->second->road_link.predecessor != nullptr)
      {
        if(it->second->road_link.predecessor->element_type == "junction")
        {
          std::vector<lane_junction_t> & options = junctionsData[it->second->road_link.predecessor->id][it->first];
          for(size_t i = 0; i < options.size(); ++i) roadSegment.predecessor_id.push_back(options[i].connection_road);
        }
        else
        {
          roadSegment.predecessor_id.push_back(it->second->road_link.predecessor->id);
        }
      }
    }

    return road::Map();
  }

  road::Map OpenDrive::Load(std::istream &input) {
    UNUSED(input);
    return road::Map();
  }
} // namespace opendrive
} // namespace carla