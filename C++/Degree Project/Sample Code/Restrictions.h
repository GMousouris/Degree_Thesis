#include "Restrictions_Defines.h"
#include "Restrictions_structs.h"
#include <optixu/optixu_math_namespace.h>

#include <string.h>
#include <string>

class Restriction
{

protected:

	static int res_id;

	Restriction_ID id;
	int r_unique_id;
	class SceneObject* object;
	class SceneObject* subject;
	optix::float2 m_angle_delta;
	optix::float2 m_angle_delta_2;
	optix::float2 m_dist_limit;

	class SceneObject* old_A = 0;
	class SceneObject* old_B = 0;
	class SceneObject* old_a = 0;
	class SceneObject* old_b = 0;
	class SceneObject* old_A2 = 0;
	class SceneObject* old_B2 = 0;
	class SceneObject* old_a2 = 0;
	class SceneObject* old_b2 = 0;
	class SceneObject* old_A3 = 0;
	class SceneObject* old_B3 = 0;
	class SceneObject* old_a3 = 0;
	class SceneObject* old_b3 = 0;

	bool is_active  = true;
	bool is_drown   = false;
	bool is_enabled = true;
	bool render_state = true;

	bool is_ui_hovered  = false;
	bool is_ui_selected = false;
	bool is_dirty    = false;
	bool is_gui_open = false;
	bool is_drawn    = false;

	bool isHit_subj = false;
	bool isHit_obj  = false;
	optix::float3 hitP_subj = optix::make_float3(0.0f);
	optix::float3 hitP_obj  = optix::make_float3(0.0f);

	bool auto_update = true;

public:

	Restriction();
	Restriction(Restriction_ID id);
	Restriction(class SceneObject* subject, class SceneObject* object, Restriction_ID id);
	Restriction(class SceneObject* subj, class SceneObject* obj, optix::float2 dist, Restriction_ID id);
	Restriction(class SceneObject* subj, class SceneObject* obj, optix::float2 dist, optix::float2 angle_delta, Restriction_ID id);
	~Restriction();

public:

	void setHit_subj(bool state)
	{
		isHit_subj = state;
	}
	void setHit_obj(bool state)
	{
		isHit_obj = state;
	}
	void set_HitPoint_Subject(optix::float3 p)
	{
		hitP_subj = p;
	}
	void set_HitPoint_Object(optix::float3 p)
	{
		hitP_obj = p;
	}
	bool get_HitPoint_Subject(optix::float3 &p)
	{
		p = hitP_subj;
		return isHit_subj;
	}
	bool get_HitPoint_Object(optix::float3& p)
	{
		p = hitP_obj;
		return isHit_obj;
	}

	bool isAutoUpdate()
	{
		return auto_update;
	}
	void setAutoUpdate(bool state)
	{
		auto_update = state;
	}


	virtual float get_Error();

	Restriction_ID     getId();
	class SceneObject* getSubject();
	class SceneObject* getObject();
	class SceneObject* get_old_Subject();
	class SceneObject* get_old_Object();
	class SceneObject* get_old_Subject2();
	class SceneObject* get_old_Object2();
	class SceneObject* get_old_Subject3();
	class SceneObject* get_old_Object3();

	class SceneObject* owner_A = nullptr;
	class SceneObject* owner_B = nullptr;

	bool temp_inactive = false;

	void set_Subject(class SceneObject* subj);
	void set_Object(class SceneObject* obj);
	void set_old_Subject(class SceneObject* subj);
	void set_old_Object(class SceneObject* obj);
	void set_old_Subject2(class SceneObject* subj);
	void set_old_Object2(class SceneObject* obj);
	void set_old_Subject3(class SceneObject* subj);
	void set_old_Object3(class SceneObject* obj);


	void set_Owner(int index, class SceneObject* owner);
	class SceneObject* get_Owner(int index);


	optix::float2 get_DistLimit();
	void          set_DistLimit(optix::float2 d);

	optix::float2 get_DeltaLimit();
	void          set_DeltaLimit(optix::float2 d);

	optix::float2 get_DeltaLimit_2();
	void          set_DeltaLimit_2(optix::float2 d);

	void set_tempInactive(bool state)
	{
		temp_inactive = state;
		if (true)
			set_Enabled(false);
	}
	bool isTempInactive()
	{
		return temp_inactive;
	}
	void set_Active(bool state);
	bool is_Active(bool state_temp_inactive = true);

	int  get_Unique_Id();
	virtual bool is_Valid( optix::float2* error = 0, bool dist_too = false );

	void mark_Drown();
	bool needs_Drow();
	void reset_Render_State();

	bool isEnabled();
	void set_Enabled(bool state);
	void toggle_State();

	void toggle_AutoUpdate()
	{
		auto_update = !auto_update;
	}

	void set_Render_State(bool state);
	bool is_Renderable();
	void toggle_Render_State();
	void set_Focused(bool state);
	void set_Selected(bool state);
	bool is_Focused();
	bool is_Selected();
	void reset_ui_states(bool only_focus = false);

	void setDirty(bool s);
	void markDirty();
	bool isDirty(bool reset = false);

	void set_GUI_Open( bool state );
	bool is_GUI_Open();

	bool isDrawn() { bool drawn = is_drawn; is_drawn = true; return drawn; }
	void setDrawn(bool state) { is_drawn = state; }

	virtual bool checkForValidState();

	std::string getNamea();
	std::string getNameb();
	std::string getNameA();
	std::string getNameB();
	int getIDa();
	int getIDb();
	int getIDA();
	int getIDB();

};