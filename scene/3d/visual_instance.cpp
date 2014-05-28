/*************************************************************************/
/*  visual_instance.cpp                                                  */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#include "visual_instance.h"

#include "servers/visual_server.h"
#include "room_instance.h"
#include "scene/scene_string_names.h"

#include "skeleton.h"

AABB VisualInstance::get_transformed_aabb() const {

	return get_global_transform().xform( get_aabb() );
}



void VisualInstance::_notification(int p_what) {

	switch(p_what) {

		case NOTIFICATION_ENTER_WORLD: {

			// CHECK ROOM
			Spatial * parent = get_parent_spatial();
			Room *room=NULL;

			while(parent) {

				room = parent->cast_to<Room>();
				if (room)
					break;
				else
					parent=parent->get_parent_spatial();
			}


			if (room) {

				VisualServer::get_singleton()->instance_set_room(instance,room->get_instance());
			}
			// CHECK SKELETON => moving skeleton attaching logic to MeshInstance
			/*
			Skeleton *skeleton=get_parent()?get_parent()->cast_to<Skeleton>():NULL;
			if (skeleton)
				VisualServer::get_singleton()->instance_attach_skeleton( instance, skeleton->get_skeleton() );
			*/

			VisualServer::get_singleton()->instance_set_scenario( instance, get_world()->get_scenario() );

		} break;
		case NOTIFICATION_TRANSFORM_CHANGED: {

			Transform gt = get_global_transform();
			VisualServer::get_singleton()->instance_set_transform(instance,gt);
		} break;
		case NOTIFICATION_EXIT_WORLD: {

			VisualServer::get_singleton()->instance_set_scenario( instance, RID() );
			VisualServer::get_singleton()->instance_set_room(instance,RID());
			VisualServer::get_singleton()->instance_attach_skeleton( instance, RID() );


		} break;
	}
}

RID VisualInstance::get_instance() const {

	return instance;
}

RID VisualInstance::_get_visual_instance_rid() const {

	return instance;
}

void VisualInstance::set_layer_mask(uint32_t p_mask) {

	layers=p_mask;
	VisualServer::get_singleton()->instance_set_layer_mask(instance,p_mask);
}

uint32_t VisualInstance::get_layer_mask() const {

	return layers;
}


void VisualInstance::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("_get_visual_instance_rid"),&VisualInstance::_get_visual_instance_rid);
	ObjectTypeDB::bind_method(_MD("set_base","base"), &VisualInstance::set_base);
	ObjectTypeDB::bind_method(_MD("set_layer_mask","mask"), &VisualInstance::set_layer_mask);
	ObjectTypeDB::bind_method(_MD("get_layer_mask"), &VisualInstance::get_layer_mask);

	ADD_PROPERTY( PropertyInfo( Variant::INT, "layers",PROPERTY_HINT_ALL_FLAGS), _SCS("set_layer_mask"), _SCS("get_layer_mask"));


}


void VisualInstance::set_base(const RID& p_base) {

	VisualServer::get_singleton()->instance_set_base(instance,p_base);
}


VisualInstance::VisualInstance()
{

	instance = VisualServer::get_singleton()->instance_create();
	VisualServer::get_singleton()->instance_attach_object_instance_ID( instance, get_instance_ID() );
	layers=1;
}


VisualInstance::~VisualInstance() {

	VisualServer::get_singleton()->free(instance);
}




void GeometryInstance::set_material_override(const Ref<Material>& p_material) {

	material_override=p_material;
	VS::get_singleton()->instance_geometry_set_material_override(get_instance(),p_material.is_valid() ? p_material->get_rid() : RID());
}

Ref<Material> GeometryInstance::get_material_override() const{

	return material_override;
}



void GeometryInstance::set_draw_range_begin(float p_dist){

	draw_begin=p_dist;
	VS::get_singleton()->instance_geometry_set_draw_range(get_instance(),draw_begin,draw_end);
}

float GeometryInstance::get_draw_range_begin() const{

	return draw_begin;
}


void GeometryInstance::set_draw_range_end(float p_dist) {

	draw_end=p_dist;
	VS::get_singleton()->instance_geometry_set_draw_range(get_instance(),draw_begin,draw_end);

}

float GeometryInstance::get_draw_range_end() const {

	return draw_end;
}

void GeometryInstance::set_flag(Flags p_flag,bool p_value) {

	ERR_FAIL_INDEX(p_flag,FLAG_MAX);
	if (flags[p_flag]==p_value)
		return;

	flags[p_flag]=p_value;
	VS::get_singleton()->instance_geometry_set_flag(get_instance(),(VS::InstanceFlags)p_flag,p_value);
	if (p_flag==FLAG_VISIBLE) {
		_change_notify("geometry/visible");
		emit_signal(SceneStringNames::get_singleton()->visibility_changed);
	}


}

bool GeometryInstance::get_flag(Flags p_flag) const{

	ERR_FAIL_INDEX_V(p_flag,FLAG_MAX,false);
	return flags[p_flag];

}


void GeometryInstance::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("set_material_override","material"), &GeometryInstance::set_material_override);
	ObjectTypeDB::bind_method(_MD("get_material_override"), &GeometryInstance::get_material_override);

	ObjectTypeDB::bind_method(_MD("set_flag","flag","value"), &GeometryInstance::set_flag);
	ObjectTypeDB::bind_method(_MD("get_flag","flag"), &GeometryInstance::get_flag);

	ObjectTypeDB::bind_method(_MD("set_draw_range_begin","mode"), &GeometryInstance::set_draw_range_begin);
	ObjectTypeDB::bind_method(_MD("get_draw_range_begin"), &GeometryInstance::get_draw_range_begin);

	ObjectTypeDB::bind_method(_MD("set_draw_range_end","mode"), &GeometryInstance::set_draw_range_end);
	ObjectTypeDB::bind_method(_MD("get_draw_range_end"), &GeometryInstance::get_draw_range_end);

	ADD_PROPERTYI( PropertyInfo( Variant::BOOL, "geometry/visible"), _SCS("set_flag"), _SCS("get_flag"),FLAG_VISIBLE);
	ADD_PROPERTY( PropertyInfo( Variant::OBJECT, "geometry/material_override",PROPERTY_HINT_RESOURCE_TYPE,"Material"), _SCS("set_material_override"), _SCS("get_material_override"));
	ADD_PROPERTYI( PropertyInfo( Variant::BOOL, "geometry/cast_shadow"), _SCS("set_flag"), _SCS("get_flag"),FLAG_CAST_SHADOW);
	ADD_PROPERTYI( PropertyInfo( Variant::BOOL, "geometry/receive_shadows"), _SCS("set_flag"), _SCS("get_flag"),FLAG_RECEIVE_SHADOWS);
	ADD_PROPERTY( PropertyInfo( Variant::INT, "geometry/range_begin",PROPERTY_HINT_RANGE,"0,32768,0.01"), _SCS("set_draw_range_begin"), _SCS("get_draw_range_begin"));
	ADD_PROPERTY( PropertyInfo( Variant::INT, "geometry/range_end",PROPERTY_HINT_RANGE,"0,32768,0.01"), _SCS("set_draw_range_end"), _SCS("get_draw_range_end"));
	ADD_PROPERTYI( PropertyInfo( Variant::BOOL, "geometry/billboard"), _SCS("set_flag"), _SCS("get_flag"),FLAG_BILLBOARD);
	ADD_PROPERTYI( PropertyInfo( Variant::BOOL, "geometry/billboard_y"), _SCS("set_flag"), _SCS("get_flag"),FLAG_BILLBOARD_FIX_Y);
	ADD_PROPERTYI( PropertyInfo( Variant::BOOL, "geometry/depth_scale"), _SCS("set_flag"), _SCS("get_flag"),FLAG_DEPH_SCALE);
	ADD_PROPERTYI( PropertyInfo( Variant::BOOL, "geometry/visible_in_all_rooms"), _SCS("set_flag"), _SCS("get_flag"),FLAG_VISIBLE_IN_ALL_ROOMS);

	ADD_SIGNAL( MethodInfo("visibility_changed"));

	BIND_CONSTANT(FLAG_VISIBLE );
	BIND_CONSTANT(FLAG_CAST_SHADOW );
	BIND_CONSTANT(FLAG_RECEIVE_SHADOWS );
	BIND_CONSTANT(FLAG_BILLBOARD );
	BIND_CONSTANT(FLAG_BILLBOARD_FIX_Y );
	BIND_CONSTANT(FLAG_DEPH_SCALE );
	BIND_CONSTANT(FLAG_VISIBLE_IN_ALL_ROOMS );
	BIND_CONSTANT(FLAG_MAX );

}

GeometryInstance::GeometryInstance() {
	draw_begin=0;
	draw_end=0;
	flags[FLAG_VISIBLE]=true;
	flags[FLAG_CAST_SHADOW]=true;
	flags[FLAG_RECEIVE_SHADOWS]=true;
	flags[FLAG_BILLBOARD]=false;
	flags[FLAG_BILLBOARD_FIX_Y]=false;
	flags[FLAG_DEPH_SCALE]=false;
	flags[FLAG_VISIBLE_IN_ALL_ROOMS]=false;

}
