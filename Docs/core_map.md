# 3rd. Maps and navigation

After discussing about the world and its actors, it is time to put everything into place and understand the map and how do the actors navigate it.  

*   [__The map__](#the-map)  
	*   [Changing the map](#changing-the-map)  
	*   [Landmarks](#landmarks)  
	*   [Lanes](#lanes)  
	*   [Junctions](#junctions)  
	*   [Waypoints](#waypoints)  
	*   [Environment Objects](#environment-objects)
*   [__Navigation in CARLA__](#navigation-in-carla)  
	*   [Navigating through waypoints](#navigating-through-waypoints)  
	*   [Generating a map navigation](#generating-a-map-navigation)  
*   [__CARLA maps__](#carla-maps)  
	*   [Non-layered maps](#non-layered-maps)
	*   [Layered maps](#layered-maps)

---
## The map

A map includes both the 3D model of a town and its road definition. Every map is based on an OpenDRIVE file describing the road layout fully annotated. The way the [OpenDRIVE standard 1.4](http://www.opendrive.org/docs/OpenDRIVEFormatSpecRev1.4H.pdf) defines roads, lanes, junctions, etc. is extremely important. It determines the possibilities of the API and the reasoning behind decisions made.  

The Python API makes for a high level querying system to navigate these roads. It is constantly evolving to provide a wider set of tools.

### Changing the map

__To change the map, the world has to change too__. Everything will be rebooted and created from scratch, besides the Unreal Editor itself. There are two ways to do so.  

* `reload_world()` creates a new instance of the world with the same map.  
* `load_world()` changes the current map and creates a new world.  

```py
world = client.load_world('Town01')
```
The client can get a list of available maps. Each map has a `name` attribute that matches the name of the currently loaded city, e.g. _Town01_.
```py
print(client.get_available_maps())
```

### Landmarks

The traffic signs defined in the OpenDRIVE file are translated into CARLA as landmark objects that can be queried from the API. In order to facilitate their manipulation, there have been several additions to it.  

*   __[carla.Landmark](https://carla.readthedocs.io/en/latest/python_api/#carla.Landmark)__ objects represent the OpenDRIVE signals. The attributes and methods describe the landmark, and where it is effective.  
	*	[__carla.LandmarkOrientation__](https://carla.readthedocs.io/en/latest/python_api/#carla.LandmarkOrientation) states the orientation of the landmark with regards of the road's geometry definition.  
	*	[__carla.LandmarkType__](https://carla.readthedocs.io/en/latest/python_api/#carla.LandmarkType) contains some common landmark types, to ease translation to OpenDRIVE types.  
*   A __[carla.Waypoint](https://carla.readthedocs.io/en/latest/python_api/#carla.Waypoint)__ can get landmarks located a certain distance ahead of it. The type of landmark can be specified. 
*   The __[carla.Map](https://carla.readthedocs.io/en/latest/python_api/#carla.Map)__ retrieves sets of landmarks. It can return all the landmarks in the map, or those having an ID, type or group in common.  
*   The __[carla.World](https://carla.readthedocs.io/en/latest/python_api/#carla.World)__ acts as intermediary between landmarks, and the *carla.TrafficSign* and *carla.TrafficLight* that embody them in the simulation.  

```py
my_waypoint.get_landmarks(200.0,True)
``` 

### Lanes

The lane types defined by [OpenDRIVE standard 1.4](http://www.opendrive.org/docs/OpenDRIVEFormatSpecRev1.4H.pdf) are translated to the API in [__carla.LaneType__](python_api.md#carla.LaneType) as a series of enum values.  

The lane markings surrounding a lane can be accessed through [__carla.LaneMarking__](python_api.md#carla.LaneMarking). These are defined with a series of variables.  

* [__carla.LaneMarkingType__](python_api.md#carla.LaneMarkingType) are enum values according to OpenDRIVE standards. 
* [__carla.LaneMarkingColor__](python_api.md#carla.LaneMarkingColor) are enum values to determine the color of the marking. 
* __width__ to state thickness of the marking. 
* [__carla.LaneChange__](python_api.md#carla.LaneChange) to state permissions to perform lane changes.  

Waypoints use these to aknowledge traffic permissions.  

```py
# Get the lane type where the waypoint is. 
lane_type = waypoint.lane_type

# Get the type of lane marking on the left. 
left_lanemarking_type = waypoint.left_lane_marking.type()

# Get available lane changes for this waypoint.
lane_change = waypoint.lane_change
```

### Junctions

A [carla.Junction](python_api.md#carla.Junction) represents an OpenDRIVE junction. This class provides for a bounding box to state whereas lanes or vehicles are inside of it.  

The most remarkable method of this class returns a pair of waypoints per lane inside the junction. Each pair is located at the starting and ending point of the junction boundaries.  

```py
waypoints_junc = my_junction.get_waypoints()
```

### Waypoints

A [__carla.Waypoint__](python_api.md#carla.Waypoint) is a 3D-directed point. These are prepared to mediate between the world and the openDRIVE definition of the road. Everything related with waypoints happens on the client-side, so there no communication with the server is needed.  

Each waypoint contains a [carla.Transform](python_api.md#carla.Transform). This states its location on the map and the orientation of the lane containing it. The variables `road_id`,`section_id`,`lane_id` and `s` translate this transform to the OpenDRIVE road. These combined, create the `id` of the waypoint.  

!!! Note
    Due to granularity, waypoints closer than __2cm within the same road__ share the same `id`. 

A waypoint also contains some information regarding the __lane__ containing it. Specifically its left and right __lane markings__, and a boolean to determine if it is inside a junction.  

```py
# Examples of a waypoint accessing to lane information
inside_junction = waypoint.is_junction()
width = waypoint.lane_width
right_lm_color = waypoint.right_lane_marking.color
```

### Environment Objects

Every object on a CARLA map has a set of associated variables which can be found [here][env_obj]. Included in these variables is a [unique ID][env_obj_id] that can be used to [toggle][toggle_env_obj] that object's visibility on the map. You can use the Python API to [fetch][fetch_env_obj] the IDs of each environment object based on their [semantic tag][semantic_tag]:

		# Get the buildings in the world
	    world = client.get_world()
		env_objs = world.get_environment_objects(carla.CityObjectLabel.Buildings)

		# Access individual building IDs and save in a set
		building_01 = env_objs[0]
		building_02 = env_objs[1]
		objects_to_toggle = {building_01.id, building_02.id}

		# Toggle buildings off
		world.enable_environment_objects(objects_to_toggle, False)
		# Toggle buildings on
		world.enable_environment_objects(objects_to_toggle, True)

See an example of distinct objects being toggled:

![toggle_objects_gif](/img/objects_small.gif)

[env_obj]: https://carla.readthedocs.io/en/latest/python_api/#carla.EnvironmentObject
[env_obj_id]: https://carla.readthedocs.io/en/latest/python_api/#carla.EnvironmentObject.id
[toggle_env_obj]: https://carla.readthedocs.io/en/latest/python_api/#carla.World.enable_environment_objects
[fetch_env_obj]: https://carla.readthedocs.io/en/latest/python_api/#carla.World.get_environment_objects
[semantic_tag]: https://carla.readthedocs.io/en/latest/python_api/#carla.CityObjectLabel


---
## Navigation in CARLA

Navigation in CARLA is managed via the waypoint API. This consists of a summary of methods in [carla.Waypoint](python_api.md#carla.Waypoint) and [carla.Map](python_api.md#carla.Map).  

All the queries happen on the client-side. The client only communicates with the server when retrieving the map object that will be used for the queries. There is no need to retrieve the map (`world.get_map()`) more than once.  


### Navigating through waypoints

Waypoints have a set of methods to connect with others and create a road flow. All of these methods follow traffic rules to determine only places where the vehicle can go.  

* `next(d)` creates a list of waypoints at an approximate distance `d` __in the direction of the lane__. The list contains one waypoint for each deviation possible. 
* `previous(d)` creates a list of waypoints waypoint at an approximate distance `d` __on the opposite direction of the lane__. The list contains one waypoint for each deviation possible.  
* `next_until_lane_end(d)` and `previous_until_lane_start(d)` returns a list of waypoints a distance `d` apart. The list goes from the current waypoint to the end and start of its lane, respectively.  
* `get_right_lane()` and `get_left_lane()` return the equivalent waypoint in an adjacent lane, if any. A lane change maneuver can be made by finding the next waypoint to the one on its right/left lane, and moving to it. 
```py
# Disable physics, in this example the vehicle is teleported.
vehicle.set_simulate_physics(False)
while True:
    # Find next waypoint 2 meters ahead.
    waypoint = random.choice(waypoint.next(2.0))
    # Teleport the vehicle.
    vehicle.set_transform(waypoint.transform)
```  

### Generating a map navigation

The instance of the map is provided by the world. It will be useful to create routes and make vehicles roam around the city and reach goal destinations.  

The following method asks the server for the XODR map file, and parses it to a [carla.Map](python_api.md#carla.Map) object. It only needs to be calle once. Maps can be quite heavy, and successive calls are unnecessary and expensive.  

```py
map = world.get_map()
```

* __Get recommended spawn points for vehicles__ pointed by developers. There is no ensurance that these spots will be free. 
```py
spawn_points = world.get_map().get_spawn_points()
```

* __Get the closest waypoint__ to a specific location or to a certain `road_id`, `lane_id` and `s` in OpenDRIVE.
```py
# Nearest waypoint on the center of a Driving or Sidewalk lane.
waypoint01 = map.get_waypoint(vehicle.get_location(),project_to_road=True, lane_type=(carla.LaneType.Driving | carla.LaneType.Sidewalk))

#Nearest waypoint but specifying OpenDRIVE parameters. 
waypoint02 = map.get_waypoint_xodr(road_id,lane_id,s)
```

* __Generate a collection of waypoints__ to visualize the city lanes. Creates waypoints all over the map, for every road and lane. All of them will be an approximate distance apart. 
```py
waypoint_list = map.generate_waypoints(2.0)
```

* __Generate road topology__. Returns a list of pairs (tuples) of waypoints. For each pair, the first element connects with the second one and both define the starting and ending point of each lane in the map.
```py
waypoint_tuple_list = map.get_topology()
```

* __Convert simulation point to geographical coordinates.__ Transforms a certain location to a [carla.GeoLocation](python_api.md#carla.GeoLocation) with latitude and longitude values. 
```py
my_geolocation = map.transform_to_geolocation(vehicle.transform)
```

* __Save road information.__ Converts the road information to OpenDRIVE format, and saves it to disk.
```py
info_map = map.to_opendrive()
```

---
## CARLA maps

There are eight towns in the CARLA ecosystem and each of those towns have two kinds of map, non-layered and layered. [Layers][layer_api] refer to the grouped objects within a map and consist of the following:

- NONE
- Buildings
- Decals
- Foliage
- Ground
- ParkedVehicles
- Particles
- Props
- StreetLights
- Walls
- All

[layer_api]: https://carla.readthedocs.io/en/latest/python_api/#carlamaplayer

### Non-layered maps

Non-layered maps are shown in the table below (click the town name to see an overhead image of the layout). All of the layers are present at all times and cannot be toggled on or off in these maps. Up until CARLA 0.9.11, these were the only kinds of map available.

!!! Note
    Users can [customize a map](tuto_A_map_customization.md) or even [create a new map](tuto_A_add_map_overview.md) to be used in CARLA.

| Town       | Summary |
| -----------| ------  |
| **[Town01](img/Town01.jpg)** | A basic town layout consisting of "T junctions".|
| **[Town02](img/Town02.jpg)** | Similar to **Town01**, but smaller.|
| **[Town03](img/Town03.jpg)** | The most complex town, with a 5-lane junction, a roundabout, unevenness, a tunnel, and more.|
| **[Town04](img/Town04.jpg)** | An infinite loop with a highway and a small town.|
| **[Town05](img/Town05.jpg)** | Squared-grid town with cross junctions and a bridge. It has multiple lanes per direction. Useful to perform lane changes.  |
| **[Town06](img/Town06.jpg)** | Long highways with many highway entrances and exits. It also has a [**Michigan left**](<https://en.wikipedia.org/wiki/Michigan_left>). |
| **[Town07](img/Town07.jpg)** | A rural environment with narrow roads, barns and hardly any traffic lights. |
| **[Town10](img/Town10.jpg)** | A city environment with different environments such as an avenue or promenade, and more realistic textures.|

### Layered maps

The layout of layered maps is the same as non-layered maps but it is possible to toggle off and on the layers of the map. There is a minimum layout that cannot be toggled off and consists of roads, sidewalks, traffic lights and traffic signs. Layered maps can be identified by the suffix `_Opt`, for example, `Town01_Opt`. With these maps it is possible to [load][load_layer] and [unload][unload_layer] layers via the Python API:

		# Load layered map for Town 01 with minimum layout plus buildings and parked vehicles
		world = client.load_world('Town01_Opt', carla.MapLayer.Buildings | carla.MapLayer.ParkedVehicles)

		# Toggle all buildings off
		world.unload_map_layer(carla.MapLayer.Buildings)

		# Toggle all buildings on	
		world.load_map_layer(carla.MapLayer.Buildings)

[load_layer]: https://carla.readthedocs.io/en/latest/python_api/#carla.World.load_map_layer
[unload_layer]: https://carla.readthedocs.io/en/latest/python_api/#carla.World.unload_map_layer

See an example of all layers being loaded and unloaded in sequence:

![map-layers](img/sublevels.gif)

<br>

---
That is a wrap as regarding maps and navigation in CARLA. The next step takes a closer look into sensors types, and the data they retrieve.  

Keep reading to learn more or visit the forum to post any doubts or suggestions that have come to mind during this reading. 
<div text-align: center>
<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="CARLA forum">
CARLA forum</a>
</p>
</div>
<div class="build-buttons">
<p>
<a href="../core_sensors" target="_blank" class="btn btn-neutral" title="4th. Sensors and data">
4th. Sensors and data</a>
</p>
</div>
</div>
