#pragma once
class game_sequence
{
public:
	game_sequence(int);
	virtual ~game_sequence(void);

	bool sequence_over;
	int sequence_number;
	clock_t sequence_start;
	virtual void render(void) =0;
	virtual void load(void);
	virtual void reset(void);
};

class splash_sequence : public game_sequence{
public:
	splash_sequence(int);
	void load(void);
	void render(void);
	void reset(void);
private:
	clock_t splashtimer;
	irr::video::ITexture *splashscreen;

};

class title_sequence : public game_sequence{
public:
	title_sequence(int);
	void load(void);
	void render(void);

private:
	irr::video::ITexture *titlescreen;
};

class playerselect_sequence : public game_sequence{
public:
	void load(void);
	playerselect_sequence(int);
	void render(void);
	void reset(void);

private:
	irr::video::ITexture *player_select_screen;
};

class playertypeselect_sequence : public game_sequence{
public:
	playertypeselect_sequence(int);
	void load(void);
	void render(void);
	void reset(void);

private:
	irr::video::ITexture *pick_one;

	map<int, int> current_selection;
	map<int, bool> confirmed_selection;
	map<int, bool> taken_selections;

	map<int, clock_t> last_move;

	void move_selection(int, int);
};

class play_start_sequence : public game_sequence{
public:
	play_start_sequence(int);

	bool init_cam;
	void load(void);
	void render(void);
	void reset(void);
private:
	irr::video::ITexture *playerstart;
	clock_t splashtimer;
};

class main_sequence : public game_sequence{
public:
	int c_response_ticker;

	main_sequence(int);
	void render(void);
};
