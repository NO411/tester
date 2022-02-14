#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <experimental/filesystem>
#include "raylib.h"

namespace fs = std::experimental::filesystem;

const std::string app = "Tester";
std::string app_status = "main_menu";

int random(int min, int max) {
	return rand() % (max - min + 1) + min;
}

void init() {
	SetTraceLogLevel(LOG_NONE);
	InitWindow(900, 600, app.c_str());
	SetTargetFPS(60);
	SetExitKey(0);
}

unsigned char colorChange(int a) {
	if (a + 50 < 255) {
		return (unsigned char)(a + 50);
	} else {
		return (unsigned char)(a - 50);
	}
}

class RectangleElement {
	public:
	Rectangle rect;
	std::vector<std::string> status;

	RectangleElement(){}
	~RectangleElement(){}

	bool isSelected() {
		return (CheckCollisionPointRec(GetMousePosition(), rect) && IsCursorOnScreen());
	}

	bool currentStatus() {
		return (std::find(status.begin(), status.end(), app_status) != status.end());
	}

	void setPos(float x, float y, float w, float h) {
		rect = {x - w / 2, y - h / 2, w, h};
	}

	Vector2 centerObj(float scale, float sizex, float sizey) {
		return {rect.x + (rect.width - sizex * scale) / 2, rect.y + (rect.height - sizey * scale) / 2};
	}
};

class Button: public RectangleElement {
	private:
	Color color_normal;
	Color color_selected;

	public:
	Color color;
	std::string name;
	bool pressed = false;

	std::string label;
	int font_size = 10;
	Vector2 text_pos;

	Texture2D texture;
	float texture_scale = 1;
	Vector2 texture_pos;

	Button(){}
	~Button(){}

	Button(const Rectangle& rec, const std::vector<std::string>& status, const std::string& name, const Color& color_normal):
	name(name), color_normal(color_normal) {
		setPos(rec.x, rec.y, rec.width, rec.height);
		this->status = status;
		this->color = color_normal;

		int r = color_normal.r, g = color_normal.g, b = color_normal.b;
		this->color_selected = {colorChange(r), colorChange(g), colorChange(b), 255};
	}

	Button(const Rectangle& rec, const std::vector<std::string>& status, const std::string& name, const Color& color_normal, const Texture2D& texture, float scale):
	Button(rec, status, name, color_normal) {
		this->texture = texture;
		texture_scale = scale;
		texture_pos = centerObj(scale, texture.width, texture.height);
	}

	Button(const Rectangle& rec, const std::vector<std::string>& status, const std::string& name, const Color& color_normal, const std::string& label, int font_size):
	Button(rec, status, name, color_normal) {
		this->label = label;
		this->font_size = font_size;
		updateTextColor();
	}

	bool isLabeled() {
		return (label != "");
	}

	void updateColor() {
		if (!isSelected()) {
			color = color_normal;
			return;
		}

		color = color_selected;

		if (IsMouseButtonPressed(0)) {
			pressed = true;
		}
	}

	void updateTextColor() {
		text_pos = centerObj(1, MeasureText(label.c_str(), font_size), font_size);
	}
};

class Textfield: public RectangleElement {
	public:
	Color color;
	std::string text;
	bool selected = false;
	bool changeable;
	int max_lines;

	Textfield(){}
	~Textfield(){}

	Textfield(const Rectangle& rec, const std::vector<std::string>& status, const bool& changeable, const int& max_lines):
	changeable(changeable), max_lines(max_lines) {
		setPos(rec.x, rec.y, rec.width, rec.height);
		this->status = status;
		if (changeable)
			color = LIME;
		else
			color = LIGHTGRAY;
	}

	void textAdd(char new_char) {
		std::string addition;
		std::string measure_text = this->text += addition;

		int last_enter = 0, _last_enter = measure_text.rfind('\n');
		if (_last_enter != std::string::npos) {
			last_enter = _last_enter;
		}

		if (MeasureText(measure_text.substr(last_enter).c_str(), 20) >= rect.width - 20) {
			if (std::count(text.begin(), text.end(), '\n') > max_lines - 2) {
				return;
			}
			addition += '\n';
		}
		addition += new_char;
		this->text += addition;
	}

	void reset() {
		text = "";
	}
};

class IndexCard {
	public:
	std::string question;
	std::string answer;

	float probability = 1.0;

	IndexCard(){}
	~IndexCard(){}

	IndexCard(const std::string& question, const std::string& answer):question(question), answer(answer){}

	void increaseProbability() {
		float ret = probability * 2;
		if (ret > 1) {
			ret = 1;
		}
		probability = ret;
	}

	void decreaseProbability() {
		probability /= 2;
	}
};

class IndexCardsStack {
	public:
	std::string title;
	std::string orig_title;
	std::vector<IndexCard> indexcards;

	IndexCardsStack(){}
	~IndexCardsStack(){}

	IndexCardsStack(const std::string& title, const std::string& orig_title, const std::vector<IndexCard>& indexcards):title(title), orig_title(orig_title), indexcards(indexcards){}

	int chooseRandomCard() {
		std::vector<int> low_prob;
		std::vector<int> high_prob;
		std::vector<int> high_prob_copy;
		std::vector<float> high_prob_probs;

		int vec_pos = -1;
		for (auto card : indexcards) {
			vec_pos++;
			if (card.probability < 1.0) {
				low_prob.push_back(vec_pos);
			} else {
				high_prob.push_back(vec_pos);
				high_prob_probs.push_back(card.probability);
			}
		}

		high_prob_copy = high_prob;

		int ret = 0;
		if (random(1, 10) >= 4) {
			bool found = false;
			while (high_prob_probs.size() > 0) {
				int pos = std::max_element(high_prob_probs.begin(), high_prob_probs.end()) - high_prob_probs.begin();
				if (random(1, 10) >= 2) {
					ret = high_prob[pos];
					found = true;
					break;
				}
				else {
					high_prob_probs.erase(high_prob_probs.begin() + pos);
					high_prob.erase(high_prob.begin() + pos);
				}
			}

			if (!found) {
				std::vector<int> vec;
				if (high_prob_copy.size() > 0) {
					vec = high_prob_copy;
				} else {
					vec = low_prob;
				}

				std::random_shuffle(vec.begin(), vec.end());
				ret = vec[0];
			}
		}
		else {
			std::vector<int> vec;
			if (low_prob.size() > 0) {
				vec = low_prob;
			} else {
				vec = high_prob_copy;
			}
				
			std::random_shuffle(vec.begin(), vec.end());
			ret = vec[0];
		}
		return ret;
	}
};

class StackButtons {
	public:
	Button title_button;
	Button delete_button;
	bool visible = true;
	
	StackButtons(){}
	~StackButtons(){}

	StackButtons(const Button& title_button, const Texture2D& trash):title_button(title_button) {
		createDeleteButton(title_button, trash);
	}

	void createDeleteButton(const Button& t_button, const Texture2D& trash_texture) {
		delete_button = {{title_button.rect.x + title_button.rect.width + 50, title_button.rect.y + 15, 30, 30}, title_button.status, title_button.name, RED, trash_texture, 1};
	}
};

void closeTester(const std::vector<IndexCardsStack>& stacks, fs::path& path) {
	for (auto & s: stacks) {
		fs::path _p(path);
		fs::path p(path);
		p /= s.orig_title;

		if (fs::exists(p)) {
			fs::remove_all(p);
			p = _p /= s.title;
		}
		
		fs::create_directory(p);
		int folder_num = 0;
		for (auto & i: s.indexcards) {
			folder_num++;
			fs::path p_copy = p;
			fs::path card(p_copy /= "card" + std::to_string(folder_num));
			fs::create_directory(card);
			fs::path question(card), answer(card);
			question /= "question.txt";
			answer /= "answer.txt";
			std::ofstream file(question);
    			file << i.question;
    			file.close();
			file.clear();
			file.open(answer);
			file << i.answer;
		}
	}

	for (const auto & p: fs::directory_iterator(path)) {
		std::string title = p.path().relative_path().filename();

		bool deleted = true;

		for (auto & s: stacks) {
			if (title == s.orig_title) {
				deleted = false;
				break;
			}
		}

		if (deleted) {
			fs::remove_all(p);
		}
	}

	CloseWindow();
}

std::string readFile(fs::path& path) {
	std::ifstream f(path, std::ios::in | std::ios::binary);
	const auto sz = fs::file_size(path);
	std::string result(sz, '\0');
	f.read(&result[0], sz);

	return result;
}

std::vector<IndexCardsStack> loadIndexCardsStacks(fs::path& path) {
	std::vector<IndexCardsStack> stacks = {};

	for (const auto & _p: fs::directory_iterator(path)) {
		IndexCardsStack new_stack = {};
		
		std::string title = _p.path().relative_path().filename();
		new_stack.title = title;
		new_stack.orig_title = title;

		for (const auto & p: fs::directory_iterator(_p)) {
			fs::path card(p.path());
			new_stack.indexcards.push_back({readFile((fs::path)card /= "question.txt"), readFile((fs::path)card /= "answer.txt")});
		}

		stacks.push_back(new_stack);
	}
	return stacks;
}

std::string formatTimer(int t) {
	std::string ret = std::to_string(t);
	int len = 2 - ret.length();
	for (int i = 0; i < len; i++) {
		ret = "0" + ret;
	}
	return ret;
}

std::string chooseRandom(std::vector<std::string> *v_ptr) {
	return v_ptr->at(random(0, v_ptr->size() - 1));
}

int main() {
	init();
        
	fs::path path(fs::current_path());
	path /= ".index_cards";

	if (!fs::exists(path)) {
		fs::create_directory(path);
	}

	Font font = GetFontDefault();

	std::vector<std::string> activities = {
		"Watch a YouTube video shorter than 10 minutes at double speed!",
		"Get a glass of water with lemon!",
		"Do 20 push-ups!",
		"Take a walk through your apartment!",
		"Do 20 squats!",
		"Hold the plank for five minutes!",
		"Eat an apple!",
		"Play \"Four Wins\" with someone!",
		"Read three pages in a book!",
		"Do nothing for 5 minutes!",
		"Clean up your room a bit!",
		"Complete a task on your to-do list!",
	};
	std::vector<std::string>* activities_ptr = &activities;
	std::string old_activity, activity;
	std::string countdown_string = "00:00";

	Color DARKBLUE2 = {58, 68, 102, 255};

	Texture2D turn_card_texture = LoadTexture("assets/turn_around.png");
	Texture2D trash_texture = LoadTexture("assets/trash.png");

	std::vector<Button> buttons = {
		{{450, 300, 200, 30}, {"main_menu"}, "new_index_cards", DARKBLUE2, "New index cards", 10},
		{{450, 350, 200, 30}, {"main_menu"}, "saved_index_cards", DARKBLUE2, "Saved index cards", 10},
		{{450, 400, 200, 30}, {"main_menu"}, "close", DARKBLUE2, "Close", 10},
		{{80, 30, 120, 25}, {"saved_index_cards", "new_index_cards", "index_cards_preview", "index_card_question", "index_card_answer"}, "back", DARKBLUE2, "Main menu", 10},
		{{80, 550, 100, 30}, {"new_index_cards"}, "save", LIME, "Save", 10},
		{{800, 500, 100, 30}, {"new_index_cards"}, "card_forward", DARKBLUE2, ">", 20},
		{{800, 550, 100, 30}, {"new_index_cards"}, "card_backward", DARKBLUE2, "<", 20},
		{{800, 450, 100, 30}, {"new_index_cards"}, "card_more", DARKBLUE2, "+", 20},
		{{800, 400, 100, 30}, {"new_index_cards"}, "card_less", DARKBLUE2, "-", 20},
		{{800, 350, 100, 30}, {"new_index_cards"}, "turn_card_around", DARKBLUE2, turn_card_texture, 0.5},
		{{600, 350, 200, 30}, {"index_cards_preview"}, "learn", LIME, "LEARN!", 10},
		{{300, 350, 200, 30}, {"index_cards_preview"}, "edit", DARKBLUE2, "Edit", 10},
		{{600, 550, 200, 30}, {"index_card_question"}, "no_idea", RED, "I have no idea", 10},
		{{300, 550, 200, 30}, {"index_card_question"}, "know_it", LIME, "I know it", 10},
		{{450, 550, 50, 30}, {"index_card_answer"}, "ok", DARKBLUE2, "OK", 10},
		{{800, 500, 100, 30}, {"saved_index_cards"}, "card_stack_forward", DARKBLUE2, ">", 20},
		{{800, 550, 100, 30}, {"saved_index_cards"}, "card_stack_backward", DARKBLUE2, "<", 20},
	};
	std::vector<Button>* buttons_ptr = &buttons;

	std::vector<Textfield> textfields = {
		{{450, 50, 300, 30}, {"new_index_cards"}, true, 1},
		{{450, 300, 500, 400}, {"new_index_cards"}, true, 13},
		{{450, 300, 500, 400}, {"index_card_question", "index_card_answer"}, false, 13},
	};
	std::vector<Textfield>* textfields_ptr = &textfields;

	std::vector<IndexCardsStack> index_cards_stacks = loadIndexCardsStacks(path);
	std::vector<IndexCardsStack>* index_cards_stacks_ptr = &index_cards_stacks;

	IndexCardsStack current_index_cards_stack = {"", "", {{"", ""}}};
	IndexCardsStack* current_index_cards_stack_ptr = &current_index_cards_stack;
	int current_stack_vector_index = 0;
	int current_question = 0;

	auto new_current_question = [&current_question, &current_index_cards_stack_ptr]() {
		current_question =  current_index_cards_stack_ptr->chooseRandomCard();
	};

	std::vector<StackButtons> stack_buttons = {};
	std::vector<StackButtons>* stack_buttons_ptr = &stack_buttons;
	int stack_buttons_page = 1;

	for (auto i = index_cards_stacks_ptr->begin(); i != index_cards_stacks_ptr->end(); ++i) {
		stack_buttons_ptr->push_back(StackButtons{Button{{200, (float)(80 + 50 * stack_buttons_ptr->size()), 300, 30}, {"saved_index_cards"}, i->title, DARKBLUE2, i->title, 20}, trash_texture});
	}

	auto update_saved_stack_list = [&stack_buttons_page, &stack_buttons_ptr, &DARKBLUE2, &trash_texture]() {
		int num = 0;
		for (auto s = stack_buttons_ptr->begin(); s != stack_buttons_ptr->end(); ++s) {
			num++;
			if (num <= stack_buttons_page * 10 && num > (stack_buttons_page * 10 - 10)) {

				s->visible = true;
				std::string title = s->title_button.label;
				s->title_button = {{200, (float)(80 + 50 * (num - stack_buttons_page * 10 + 9)), 300, 30}, {"saved_index_cards"}, title, DARKBLUE2, title, 20};
				s->createDeleteButton(s->title_button, trash_texture);
			} else {
				s->visible = false;
			}
		}
	};

	std::string page = "0 - Front";

	bool please_change_title = false;
	bool new_current_stack = true;
	bool change_vec_index = true;
	bool interact = true;

	float timer = 0, blink_timer = 0, change_title_timer = 0, interact_timer = 0, sec_timer = 0;
	int countdown = 300;
	double wait_time = GetTime();

	while (!WindowShouldClose()) {
		float dtime = GetFrameTime();
		timer += dtime;
		blink_timer += dtime;
		change_title_timer += dtime;
		interact_timer += dtime;
		sec_timer += dtime;

		if (change_title_timer >= 2) {
			change_title_timer = 0;
			please_change_title = false;
		}

		if (!interact && sec_timer >= 1) {
			countdown--;
			int min = countdown / 60;
			int sec = countdown - min * 60;

			countdown_string = formatTimer(min) + ":" + formatTimer(sec);
			sec_timer = 0;
		}

		if (interact_timer >= 25 * 60 && interact) {
			interact = false;
			interact_timer = 0;
			HideCursor();

			activity = chooseRandom(activities_ptr);
			while (old_activity == activity) {
				activity = chooseRandom(activities_ptr);
			}

			old_activity = activity;
		} else if (interact_timer >= 5 * 60 && !interact) {
			interact = true;
			interact_timer = 0;
			ShowCursor();
			countdown = 300;
		}

		int card_number = std::stoi(page.substr(0, page.find('-') - 1));
		int _find = page.find('-') + 2;
		std::string front_or_back = page.substr(_find);
		bool cursor_in_textfield = false;

		if (interact) {
			auto save_card_entry = [&front_or_back, &current_index_cards_stack_ptr, &card_number, &textfields_ptr]() {
				if (front_or_back == "Front") {
					current_index_cards_stack_ptr->indexcards.at(card_number).question = textfields_ptr->at(1).text;
					return;
				}

				current_index_cards_stack_ptr->indexcards.at(card_number).answer = textfields_ptr->at(1).text;
			};

			for (auto n = buttons_ptr->begin(); n != buttons_ptr->end(); ++n) {
				if (n->currentStatus()) {
					n->updateColor();

					if (IsMouseButtonReleased(0) && n->pressed) {
						std::string name = n->name;
						if (name == "new_index_cards") {
							new_current_stack = true;
							change_vec_index = true;
							app_status = "new_index_cards";
						} else if (name == "saved_index_cards") {
							app_status = "saved_index_cards";
							stack_buttons_page = 1;
							update_saved_stack_list();
						} else if (name == "back") {
							app_status = "main_menu";
							current_index_cards_stack = {"", "", {{"", ""}}};
							page = "0 - Front";
							textfields_ptr->at(1).reset();
							textfields_ptr->at(0).reset();
						} else if (name == "turn_card_around") {
							save_card_entry();
							if (front_or_back == "Front") {
								page.replace(_find, 5, "Back");
								textfields_ptr->at(1).text = current_index_cards_stack_ptr->indexcards.at(card_number).answer;
							} else {
								page.replace(_find, 4, "Front");
								textfields_ptr->at(1).text = current_index_cards_stack_ptr->indexcards.at(card_number).question;
							}
						} else if (name == "card_less" && card_number > 0) {
							page = std::to_string(card_number - 1) + " - Front";
							current_index_cards_stack_ptr->indexcards.erase(current_index_cards_stack_ptr->indexcards.begin() + card_number);
							textfields_ptr->at(1).text = current_index_cards_stack_ptr->indexcards.at(card_number - 1).question;
						} else if (name == "card_more") {
							save_card_entry();

							page = std::to_string(card_number + 1) + " - Front";
							current_index_cards_stack_ptr->indexcards.insert(current_index_cards_stack_ptr->indexcards.begin() + card_number + 1, {"", ""});
							textfields_ptr->at(1).text = current_index_cards_stack_ptr->indexcards.at(card_number + 1).question;
						} else if (name == "card_forward" && current_index_cards_stack_ptr->indexcards.size() > card_number + 1) {
							save_card_entry();
							page = std::to_string(card_number + 1) + " - Front";
							textfields_ptr->at(1).text = current_index_cards_stack_ptr->indexcards.at(card_number + 1).question;
						} else if (name == "card_backward" && card_number > 0) {
							save_card_entry();
							page = std::to_string(card_number - 1) + " - Front";
							textfields_ptr->at(1).text = current_index_cards_stack_ptr->indexcards.at(card_number - 1).question;
						} else if (name == "save") {

							std::string title = textfields_ptr->at(0).text;

							bool same_title = false;

							auto set_same_title = [&same_title, &please_change_title, &change_title_timer]() {
								same_title = true;
								please_change_title = true;
								change_title_timer = 0;
							};

							for (auto s = index_cards_stacks_ptr->begin(); s != index_cards_stacks_ptr->end(); ++s) {
								if (s->title == title && new_current_stack) {
									set_same_title();
									break;
								}
							}

							if (title == "") {
								set_same_title();
							}

							if (!same_title) {
								save_card_entry();

								app_status = "index_cards_preview";
								current_index_cards_stack_ptr->title = title;

								if (new_current_stack) {
									current_index_cards_stack_ptr->orig_title = title;
									index_cards_stacks_ptr->push_back(*current_index_cards_stack_ptr);
									stack_buttons_ptr->push_back(StackButtons{Button{{200, (float)(80 + 50 * stack_buttons_ptr->size()), 300, 30}, {"saved_index_cards"}, title, DARKBLUE2, title, 20}, trash_texture});
								} else {

									if (change_vec_index) {
										for (auto d = stack_buttons_ptr->begin(); d != stack_buttons_ptr->end(); ++d) {
											int vec_pos = -1;
											for (auto s = index_cards_stacks_ptr->begin(); s != index_cards_stacks_ptr->end(); ++s) {
												vec_pos++;
												if (s->title == d->title_button.name) {
													current_stack_vector_index = vec_pos;
													break;
												}
											}
										}
									}

									index_cards_stacks_ptr->at(current_stack_vector_index) = *current_index_cards_stack_ptr;
									Button* b = &stack_buttons_ptr->at(current_stack_vector_index).title_button;
									b->name = current_index_cards_stack_ptr->title;
									b->label = current_index_cards_stack_ptr->title;
									b->updateTextColor();
								}
							}
						} else if (name == "learn") {
							new_current_question();
							textfields_ptr->at(2).text = current_index_cards_stack_ptr->indexcards.at(current_question).question;
							app_status = "index_card_question";
						} else if (name == "edit") {
							app_status = "new_index_cards";
							new_current_stack = false;
						} else if (name == "know_it") {
							textfields_ptr->at(2).text = current_index_cards_stack_ptr->indexcards.at(current_question).answer;
							current_index_cards_stack_ptr->indexcards.at(current_question).decreaseProbability();
							app_status = "index_card_answer";
						} else if (name == "no_idea") {
							textfields_ptr->at(2).text = current_index_cards_stack_ptr->indexcards.at(current_question).answer;
							current_index_cards_stack_ptr->indexcards.at(current_question).increaseProbability();
							app_status = "index_card_answer";
						} else if (name == "ok") {
							new_current_question();
							textfields_ptr->at(2).text = current_index_cards_stack_ptr->indexcards.at(current_question).question;
							app_status = "index_card_question";
						} else if (name == "card_stack_forward" && stack_buttons_ptr->size() > stack_buttons_page * 10) {
							stack_buttons_page++;
							update_saved_stack_list();
						} else if (name == "card_stack_backward" && stack_buttons_page > 1) {
							stack_buttons_page--;
							update_saved_stack_list();
						} else if (name == "close") {
							closeTester(index_cards_stacks, path);
						}

						n->pressed = false;
					}
				}
			}

			int _v_pos = -2;
			for (auto n = stack_buttons_ptr->begin(); n != stack_buttons_ptr->end(); ++n) {
				if (n->title_button.currentStatus() && n->visible) {
					n->title_button.updateColor();
					n->delete_button.updateColor();

					if (IsMouseButtonReleased(0) && n->title_button.pressed) {
						int vec_pos = -1;
						for (auto s = index_cards_stacks_ptr->begin(); s != index_cards_stacks_ptr->end(); ++s) {
							vec_pos++;
							if (s->title == n->title_button.name) {
								textfields_ptr->at(0).text = s->title;
								textfields_ptr->at(1).text = s->indexcards.at(0).question;
								current_index_cards_stack = {s->title, s->title, s->indexcards};
								current_stack_vector_index = vec_pos;
								break;
							}
						}
						change_vec_index = false;
						app_status = "index_cards_preview";
						n->title_button.pressed = false;
					} else if (IsMouseButtonReleased(0) && n->delete_button.pressed) {
						int v_pos = -1;
						for (auto i = index_cards_stacks_ptr->begin(); i != index_cards_stacks_ptr->end(); ++i) {
							v_pos++;
							if (i->title == n->delete_button.name) {
								_v_pos = v_pos;
								break;
							}
						}
						n->delete_button.pressed = false;
					}

				}
			}
			if (_v_pos > -2) {
				index_cards_stacks_ptr->erase(index_cards_stacks_ptr->begin() + _v_pos);
				stack_buttons_ptr->erase(stack_buttons_ptr->begin() + _v_pos);
				update_saved_stack_list();
			}

			for (auto t = textfields_ptr->begin(); t != textfields_ptr->end(); ++t) {
				if (t->currentStatus()) {
					if (t->isSelected() && t->changeable) {
						if (IsMouseButtonPressed(0)) {
							for (auto t2 = textfields_ptr->begin(); t2 != textfields_ptr->end(); ++t2) {
								if (t2->changeable) {
									t2->selected = false;
								}
							}
							t->selected = true;
						}
					}

					if (t->selected) {
						if (t->isSelected()) {
							cursor_in_textfield = true;
						}

						int max_length = 100;
						int char_key = GetCharPressed();

        					while (char_key > 0) {
        						if ((char_key >= 32) && (char_key <= 125)) {
								t->textAdd((char)char_key);
        					   	}
        					    	char_key = GetCharPressed();
        					}

						int key = GetKeyPressed();
						switch (key) {
							case KEY_BACKSPACE:
								t->text = t->text.substr(0, t->text.size() - 1);
								wait_time = GetTime();
								break;
							case KEY_ENTER:
								if (std::count(t->text.begin(), t->text.end(), '\n') < t->max_lines - 1)
								{
									t->text += '\n';
								}
								wait_time = GetTime();
								break;
							case KEY_TAB:
								t->textAdd('\t');
								wait_time = GetTime();
								break;
							default:
								break;
						}

						if (timer >= 0.05 && GetTime() >= wait_time + 0.6) {
							if (IsKeyDown(KEY_BACKSPACE)) {
								t->text = t->text.substr(0, t->text.size() - 1);
							} else if (IsKeyDown(KEY_ENTER) && std::count(t->text.begin(), t->text.end(), '\n') < t->max_lines - 1) {
								t->text += "\n";
							}

							timer = 0;
						}
					}
				}
			}

			if (cursor_in_textfield && !IsCursorHidden()) {
				HideCursor();
			} else if (!cursor_in_textfield && IsCursorHidden()) {
				ShowCursor();
			}


			if (IsMouseButtonPressed(0)) {
				bool textfield_pressed = false;
				for (auto t = textfields_ptr->begin(); t != textfields_ptr->end(); ++t) {
					if (t->currentStatus() && t->isSelected()) {
						textfield_pressed = true;
						break;
					}
				}
				if (!textfield_pressed) {
					for (auto t = textfields_ptr->begin(); t != textfields_ptr->end(); ++t) {
						t->selected = false;
					}
				}
			}
		}

		BeginDrawing();
			ClearBackground(RAYWHITE);

			for (auto n = buttons_ptr->begin(); n != buttons_ptr->end(); ++n) {
				if (n->currentStatus()) {
					DrawRectangleRoundedLines(n->rect, 0.5f, 0, 5, n->color);
					if (n->isLabeled()) {
						DrawText(n->label.c_str(), n->text_pos.x, n->text_pos.y, n->font_size, n->color);
					} else {
						DrawTextureEx(n->texture, n->texture_pos, 0.0f, n->texture_scale, n->color);
					}
				}
			}

			for (auto n = stack_buttons_ptr->begin(); n != stack_buttons_ptr->end(); ++n) {
				if (n->title_button.currentStatus() && n->visible) {
					DrawRectangleLinesEx(n->title_button.rect, 3.0f, n->title_button.color);
					DrawText(n->title_button.label.c_str(), n->title_button.text_pos.x, n->title_button.text_pos.y, n->title_button.font_size, n->title_button.color);
					DrawRectangleRoundedLines(n->delete_button.rect, 0.5f, 0, 5, n->delete_button.color);
					DrawTextureEx(n->delete_button.texture, n->delete_button.texture_pos, 0.0f, n->delete_button.texture_scale, n->delete_button.color);
				}
			}

			for (auto t = textfields_ptr->begin(); t != textfields_ptr->end(); ++t) {
				if (t->currentStatus()) {
					DrawRectangleLinesEx(t->rect, 2, t->color);
					std::string draw_text = t->text;
					
					if (t->selected && blink_timer >= 0.5) {
						draw_text += '|';
					}

					if (blink_timer >= 1) {
						blink_timer = 0;
					}

					DrawText(draw_text.c_str(), t->rect.x + 5, t->rect.y + 5, 20, BLACK);
				}
			}

			if (app_status == "main_menu") {
				DrawText(app.c_str(), 450 - MeasureText(app.c_str(), 50) / 2, 100, 50, DARKBLUE2);
			} else if (app_status == "index_cards_preview" || app_status == "index_card_question" || app_status == "index_card_answer") {
				std::string title = current_index_cards_stack_ptr->title;
				DrawText(title.c_str(), 450 - MeasureText(title.c_str(), 50) / 2, 40, 50, GRAY);
			} else if (app_status == "new_index_cards") {
				std::string text = std::to_string(card_number + 1) + " - " + front_or_back;
				DrawText(text.c_str(), 700 - MeasureText(text.c_str(), 20), 510, 20, GRAY);

				int cards = current_index_cards_stack_ptr->indexcards.size();
				text = std::to_string(cards) + " Card";
				if (cards > 1) {
					text += "s";
				}

				DrawText(text.c_str(), 200, 510, 20, GRAY);
				
				if (please_change_title) {
					DrawText("Already chosen, please choose a new title", 300, 70, 10, RED);
				}
			} else if (app_status == "saved_index_cards") {
				int pages = ((stack_buttons_ptr->size() - 1) / 10) + 1;
				if (pages < 1) {
					pages = 1;
				}
				std::string text = std::to_string(stack_buttons_page) + '/' + std::to_string(pages);
				DrawText(text.c_str(), 800 - MeasureText(text.c_str(), 20) / 2, 450, 20, BLACK);
			}
			

			if (cursor_in_textfield) {
				DrawText("I", GetMouseX(), GetMouseY(), 25, GRAY);
			}

			if (!interact) {
				Color clr = DARKBLUE2;
				clr.a = 250;
				DrawRectangleRec({0, 0, 900, 600}, clr);
				DrawText(countdown_string.c_str(), 450 - MeasureText(countdown_string.c_str(), 100) / 2, 150, 100, WHITE);
				DrawText(activity.c_str(), 450 - MeasureText(activity.c_str(), 20) / 2, 300, 20, WHITE);
			}

		EndDrawing();
	}
	UnloadTexture(turn_card_texture);
	UnloadTexture(trash_texture);
	closeTester(index_cards_stacks, path);
	return 0;
}
