#pragma once

#ifdef BALLOON_EXPORTS
#define BALLOON_API __declspec(dllexport)
#else 
#define BALLOON_API __declspec(dllimport)
#endif

#include <random>

constexpr float scr_width{ 1000.0f };
constexpr float scr_height{ 800.0f };
constexpr float sky{ 50.0f };
constexpr float ground{ 750.0f };

enum class dirs { up = 0, down = 1, left = 2, right = 3, up_left = 4, up_right = 5, down_left = 6, down_right = 7, stop = 8 };

enum class nature { cloud1 = 0, cloud2 = 1, sun = 2, field1 = 3, field2 = 4 };
enum class evils { bird1 = 0, bird2 = 1, bird3 = 2, gorilla = 3 };

struct BALLOON_API FPOINT
{
	float x{ 0 };
	float y{ 0 };
};

struct BALLOON_API FRECT
{
	float left{ 0 };
	float up{ 0 };
	float right{ 0 };
	float down{ 0 };
};

namespace dll
{
	class BALLOON_API RANDIT
	{
	private:
		std::mt19937* twister{ nullptr };

	public:

		RANDIT();
		~RANDIT();

		int operator()(int min, int max);
		float operator()(float min, float max);
	};

	class BALLOON_API PROTON
	{
	protected:
		float _width{ 0 };
		float _height{ 0 };

		bool _in_heap = false;

	public:
		FPOINT start{};
		FPOINT end{};
		FPOINT center{};

		float x_rad{ 0 };
		float y_rad{ 0 };

		PROTON();
		PROTON(float _sx, float _sy);
		PROTON(float _sx, float _sy, float _s_width, float _s_height);

		virtual ~PROTON() {};

		static PROTON* create(float sx, float sy, float s_width, float s_height);

		float get_width()const;
		float get_height()const;
		bool proton_in_heap()const;

		void set_edges();
		void set_width(float new_width);
		void set_height(float new_height);
		void set_dims(float new_width, float new_height);

		virtual void Release();
	};

	class BALLOON_API FIELDS :public PROTON
	{
	private:
		float _speed{ 0 };
		nature _type{ nature::sun };

		FIELDS(nature _what_type, float _first_x, float _first_y);

	public:
		dirs dir = dirs::stop;

		nature get_type()const;

		bool move(float gear);

		void Release()override;

		static FIELDS* create(nature what_type, float first_x, float first_y);
	};

	class BALLOON_API BALLOON :public PROTON
	{
	private:
		float _speed{ 5.0f };

		int frame{ 0 };
		int frame_delay{ 4 };

		BALLOON(float _first_x, float _first_y);

	public:
		dirs dir = dirs::stop;

		void move(float gear);

		void Release()override;
	
		int get_frame();

		static BALLOON* create(float first_x, float first_y);
	};

	class BALLOON_API EVILS :public PROTON
	{
	protected:
		float _speed = 1.0f;

		RANDIT _rand{};

		int max_frames{ 0 };
		int frame_delay{ 0 };
		int max_frame_delay = 0;
		int frame{ 0 };

		int attack_delay = 100;
		int max_attack_delay = 100;

		bool hor_dir = false;
		bool ver_dir = false;

		float slope{ 0 };
		float intercept{ 0 };
		float sx{ 0 };
		float sy{ 0 };
		float ex{ 0 };
		float ey{ 0 };

		evils _type{ evils::gorilla };

		dirs move_dir{ dirs::left };

		EVILS(float _first_x, float _first_y);

		void SetPath(float _target_x, float _target_y);

	public:

		virtual ~EVILS() {};

		int get_frame();
		evils get_type()const;
		dirs get_move_dir()const;
		void set_move_dir(dirs new_dir);

		virtual bool move(float gear) = 0;
		virtual void Release() = 0;
		virtual bool attack() = 0;

		static EVILS* create(evils what_type, float first_x, float first_y);
	};

	class BALLOON_API GORRILLA :public EVILS
	{
	public:
		GORRILLA(float _start_x, float _start_y);

		bool move(float gear)override;
		void Release()override;
		bool attack()override;
	};

	class BALLOON_API BIRD1 :public EVILS
	{
	public:
		BIRD1(float _start_x, float _start_y);
		bool move(float gear)override;
		void Release()override;
		bool attack()override;
	};

	class BALLOON_API BIRD2 :public EVILS
	{
	public:
		BIRD2(float _start_x, float _start_y);
		bool move(float gear)override;
		void Release()override;
		bool attack()override;
	};

	class BALLOON_API BIRD3 :public EVILS
	{
	public:
		BIRD3(float _start_x, float _start_y);
		bool move(float gear)override;
		void Release()override;
		bool attack()override;
	};

	class BALLOON_API BANNANA:public PROTON
	{
	protected:
		float sx{ 0 };
		float ex{ 0 };
		float sy{ 0 };
		float ey{ 0 };

		bool hor_dir = false;
		bool ver_dir = false;

		float slope{ 0 };
		float intercept{ 0 };

		BANNANA(float _start_x, float _start_y, float _target_x, float _target_y);

		void SetPath(float _end_x, float _end_y);

	public:
		
		bool move(float gear);
		void Release();

		static BANNANA* create(float start_x, float start_y, float target_x, float target_y);
	};

	// FUNCTIONS ***************************

	BALLOON_API bool Intersect(FRECT first, FRECT second);
	BALLOON_API bool Intersect(FPOINT first_center, FPOINT second_center, float first_x_rad, float second_x_rad,
		float first_y_rad, float second_y_rad);
}