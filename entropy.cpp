#include <SFML/Graphics.hpp>
#include <iostream>
#include <queue>
#include <string>
typedef std::pair<int, int> pairii;
using namespace sf;

const int window_width = 640 * 2, only_particle_window_height = 320 * 2, window_height = only_particle_window_height + 50, particle_size = 8, divisor = 11111, frame_limit = 10000, max_temp = 1275, temp_booster = 1;
int minimal_temp_diff = 25, new_even_temp, new_min_random_temp, new_max_random_temp, digit;
bool start = 0, editor_mode=0, text_display=0, even_temp_choice=0, random_temp_choice=0, minimal_rand_temp_done=0;
Text bottom_text;
Font sansation_bold_font;

struct Grid_cell {
    CircleShape tile;
    Vector2<int> position;
    bool neighbours_list[4];            //up right down left
    int temp;
};

RenderTexture main_render_texture, editor_mode_texture;
Sprite main_sprite,editor_mode_sprite;
Vector2<int> particle_1, particle_2, mouse_click_pos_grid_index, last_mouse_cursor_grid_index;
Color color, transparent_rect_color;
CircleShape particle;
RectangleShape rect_for_particles_editing;
Grid_cell grid[window_width / particle_size][only_particle_window_height / particle_size];
bool whether_in_queue[window_width / particle_size][only_particle_window_height / particle_size][4];
std::priority_queue < std::pair<pairii, pairii>> pqueue_for_next_move;              // {{rand_val,neighbour_index}{posx,posy}}


Color update_color_with_temp(int temp) {
    color.r=0;
    color.g = 0;
    color.b = 0;
    temp /= temp_booster;
    temp--;
    if (temp / 255 == 0) {
        temp++;
        color.b = temp;
    }
        
    else if (temp / 255 == 1) {
        temp++;
        color.b = 255;
        color.r = (temp - 255);
    }
    else if (temp / 255 == 2) {
        temp++;
        color.r = 255;
        color.b = (3*255-temp);
    }
        
    else if (temp / 255 == 3) {
        temp++;
        color.r = 255;
        color.g = (temp - 3*255);
    }
    else if (temp / 255 == 4) {
        temp++;
        color.r = 255;
        color.g = 255;
        color.b = (temp - 4 * 255);
    }
    return color;
}

Vector2<int> get_neighbour_pos(int posx, int posy, int neighbour_index) {
    if (neighbour_index == 0)
        posy--;
    else if (neighbour_index == 1)
        posx++;
    else if (neighbour_index == 2)
        posy++;
    else if (neighbour_index == 3)
        posx--;
    return { posx,posy };
}

void check_temp_with_neighbours(int posx, int posy) {
    for (int k = 0; k < 4; k++)
        if (grid[posx][posy].neighbours_list[k]) {
            Vector2 <int> neighbour_pos = get_neighbour_pos(posx, posy, k);
            if (abs(grid[posx][posy].temp - grid[neighbour_pos.x][neighbour_pos.y].temp) > minimal_temp_diff && !whether_in_queue[posx][posy][k]) {
                pqueue_for_next_move.push({ {1 + (rand() % divisor),k},{posx,posy} });
                whether_in_queue[posx][posy][k] = 1;
                whether_in_queue[neighbour_pos.x][neighbour_pos.y][(k + 2) % 4] = 1;
            }
        }
}

void initial_render() {
    for (int i=0; i< window_width / particle_size; i++)
        for (int j = 0; j < only_particle_window_height / particle_size; j++) {
            grid[i][j].temp = 700 * temp_booster;
            
            grid[i][j].tile = particle;
            grid[i][j].tile.setFillColor(update_color_with_temp(grid[i][j].temp));       
            grid[i][j].position = { i * particle_size,j * particle_size };
            grid[i][j].tile.setPosition(grid[i][j].position.x, grid[i][j].position.y);
            main_render_texture.draw(grid[i][j].tile);
            if (i < (window_width / particle_size) - 1) {
                grid[i][j].neighbours_list[1]=1;
                grid[i + 1][j].neighbours_list[3]=1;
            }
            if (j < (only_particle_window_height / particle_size) - 1) {
                grid[i][j].neighbours_list[2] = 1;
                grid[i][j + 1].neighbours_list[0] = 1;
            }
        }

    for (int i = 0; i < window_width / particle_size; i++)
        for (int j = 0; j < only_particle_window_height / particle_size; j++)
            check_temp_with_neighbours(i, j);
}

void prepare_initial_state() {
    particle.setRadius(particle_size / 2);
    
    transparent_rect_color = Color::Green;
    transparent_rect_color.a = 100;
    rect_for_particles_editing.setOutlineColor(Color::Green);
    rect_for_particles_editing.setOutlineThickness(1);
    rect_for_particles_editing.setFillColor(transparent_rect_color);

    sansation_bold_font.loadFromFile("Sansation_Bold.ttf");
    bottom_text.setFont(sansation_bold_font);
    bottom_text.setCharacterSize(20);
    bottom_text.setFillColor(Color::Green);

    initial_render();
}

void temperature_transfer() {
    int temp_sum = grid[particle_1.x][particle_1.y].temp + grid[particle_2.x][particle_2.y].temp;
    if (temp_sum % 2 == 1) {
        grid[particle_1.x][particle_1.y].tile.setFillColor(update_color_with_temp(temp_sum /2+1));
        grid[particle_1.x][particle_1.y].temp = temp_sum /2+1;
    }
    else {
        grid[particle_1.x][particle_1.y].tile.setFillColor(update_color_with_temp(temp_sum / 2));
        grid[particle_1.x][particle_1.y].temp = temp_sum / 2;
    }

    grid[particle_2.x][particle_2.y].tile.setFillColor(update_color_with_temp(temp_sum / 2));
    grid[particle_2.x][particle_2.y].temp = temp_sum /2;
    main_render_texture.draw(grid[particle_1.x][particle_1.y].tile);
    main_render_texture.draw(grid[particle_2.x][particle_2.y].tile);

    check_temp_with_neighbours(particle_1.x, particle_1.y);
    check_temp_with_neighbours(particle_2.x, particle_2.y);
}

void set_particles_to_change() {
    if (pqueue_for_next_move.empty())
        return;
    int neighbour_index;
    particle_1.x = pqueue_for_next_move.top().second.first;
    particle_1.y = pqueue_for_next_move.top().second.second;
    neighbour_index = pqueue_for_next_move.top().first.second;
    particle_2 = get_neighbour_pos(particle_1.x, particle_1.y, neighbour_index);
    pqueue_for_next_move.pop();
    whether_in_queue[particle_1.x][particle_1.y][neighbour_index] = 0;
    whether_in_queue[particle_2.x][particle_2.y][(neighbour_index+2)%4] = 0;
    if (abs(grid[particle_1.x][particle_1.y].temp - grid[particle_2.x][particle_2.y].temp) < (minimal_temp_diff)) {
        set_particles_to_change();
        return;
    }
    temperature_transfer();
}


int main() {
    srand(time(0));
    RenderWindow window(VideoMode(window_width, window_height), "Entropy");
    window.setFramerateLimit(frame_limit);

    main_render_texture.create(window_width, window_height);
    editor_mode_texture.create(window_width, window_height);
    prepare_initial_state();
    main_sprite.setTexture(main_render_texture.getTexture());
    editor_mode_sprite.setTexture(editor_mode_texture.getTexture());
    editor_mode_sprite.setPosition(0, 0);
    main_sprite.setPosition(0, 0);

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed || (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape))
                window.close();

            if (!editor_mode && event.type == Event::KeyPressed && event.key.code == Keyboard::Enter)
                if (!start)
                    start = 1;
                else
                    start = 0;

            if (!text_display && !start && event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left
                && Mouse::getPosition(window).x >= 0 && Mouse::getPosition(window).x < window_width && Mouse::getPosition(window).y >= 0 && Mouse::getPosition(window).y < only_particle_window_height) {
                editor_mode = 1;
                mouse_click_pos_grid_index = { Mouse::getPosition(window).x / particle_size,Mouse::getPosition(window).y / particle_size };
            }

            if (!text_display && !start && event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Left) {
                bottom_text.setString("Press 'R' to choose random temperature      ---     Press 'E' to choose even temperature");
                bottom_text.setPosition({ -1 + window_width / 2 - bottom_text.getGlobalBounds().width / 2,-6 + (only_particle_window_height + window_height) / 2 - bottom_text.getGlobalBounds().height / 2 });
                text_display = 1;
                new_even_temp = 0, new_min_random_temp = 0, new_max_random_temp = 0;
            }

            if (!even_temp_choice && !random_temp_choice && text_display && event.type == Event::KeyPressed && event.key.code == Keyboard::E) {
                even_temp_choice = 1;
                bottom_text.setString("Enter particles temperature from 0 to 1275 K and press 'Enter'.  ---  Current chosen temperature: " + std::to_string(new_even_temp));
                bottom_text.setPosition({ -1 + window_width / 2 - bottom_text.getGlobalBounds().width / 2,-6 + (only_particle_window_height + window_height) / 2 - bottom_text.getGlobalBounds().height / 2 });
            }

            if (!even_temp_choice && !random_temp_choice && text_display && event.type == Event::KeyPressed && event.key.code == Keyboard::R) {
                random_temp_choice = 1;
                minimal_rand_temp_done = 0;
                bottom_text.setString("Enter minimal particles temperature from 0 to 1275 K and press 'Enter'.  ---  Current chosen temperature: " + std::to_string(new_min_random_temp));
                bottom_text.setPosition({ -1 + window_width / 2 - bottom_text.getGlobalBounds().width / 2,-6 + (only_particle_window_height + window_height) / 2 - bottom_text.getGlobalBounds().height / 2 });
            }

            if (even_temp_choice && text_display) {
                if (event.type == Event::KeyPressed && (event.key.code == Keyboard::Num1 || event.key.code == Keyboard::Num2 || event.key.code == Keyboard::Num3 || event.key.code == Keyboard::Num4
                    || event.key.code == Keyboard::Num5 || event.key.code == Keyboard::Num6 || event.key.code == Keyboard::Num7 || event.key.code == Keyboard::Num8 || event.key.code == Keyboard::Num9 || (event.key.code == Keyboard::Num0 && new_even_temp != 0))) {
                    digit = event.key.code - 26;
                    if (new_even_temp * 10 + digit <= max_temp) {
                        new_even_temp = new_even_temp * 10 + digit;
                        bottom_text.setString("Enter particles temperature from 0 to 1275 K and press 'Enter'.  ---  Current chosen temperature: " + std::to_string(new_even_temp));
                    }
                }
                if (event.type == Event::KeyPressed && event.key.code == Keyboard::BackSpace && new_even_temp > 0) {
                    new_even_temp -= (new_even_temp % 10);
                    new_even_temp /= 10;
                    bottom_text.setString("Enter particles temperature from 0 to 1275 K and press 'Enter'.  ---  Current chosen temperature: " + std::to_string(new_even_temp));
                }
                if (event.type == Event::KeyPressed && event.key.code == Keyboard::Enter) {
                    for (int i = std::min(last_mouse_cursor_grid_index.x, mouse_click_pos_grid_index.x); i <= std::max(last_mouse_cursor_grid_index.x, mouse_click_pos_grid_index.x); i++)
                        for (int j = std::min(last_mouse_cursor_grid_index.y, mouse_click_pos_grid_index.y); j <= std::max(last_mouse_cursor_grid_index.y, mouse_click_pos_grid_index.y); j++)
                            grid[i][j].temp = new_even_temp * temp_booster;

                    for (int i = std::min(last_mouse_cursor_grid_index.x, mouse_click_pos_grid_index.x); i <= std::max(last_mouse_cursor_grid_index.x, mouse_click_pos_grid_index.x); i++)
                        for (int j = std::min(last_mouse_cursor_grid_index.y, mouse_click_pos_grid_index.y); j <= std::max(last_mouse_cursor_grid_index.y, mouse_click_pos_grid_index.y); j++)
                            for (int k = 0; k < 4; k++)
                                whether_in_queue[i][j][k] = 0;

                    for (int i = std::min(last_mouse_cursor_grid_index.x, mouse_click_pos_grid_index.x); i <= std::max(last_mouse_cursor_grid_index.x, mouse_click_pos_grid_index.x); i++)
                        for (int j = std::min(last_mouse_cursor_grid_index.y, mouse_click_pos_grid_index.y); j <= std::max(last_mouse_cursor_grid_index.y, mouse_click_pos_grid_index.y); j++) {
                            grid[i][j].tile.setFillColor(update_color_with_temp(grid[i][j].temp));
                            main_render_texture.draw(grid[i][j].tile);
                            check_temp_with_neighbours(i, j);
                        }

                    editor_mode = 0;
                    even_temp_choice = 0;
                    text_display = 0;
                }

            }

            if (random_temp_choice && text_display) {
                if (minimal_rand_temp_done && event.type == Event::KeyPressed && (event.key.code == Keyboard::Num1 || event.key.code == Keyboard::Num2 || event.key.code == Keyboard::Num3 || event.key.code == Keyboard::Num4
                    || event.key.code == Keyboard::Num5 || event.key.code == Keyboard::Num6 || event.key.code == Keyboard::Num7 || event.key.code == Keyboard::Num8 || event.key.code == Keyboard::Num9 || (event.key.code == Keyboard::Num0 && new_max_random_temp != 0))) {
                    digit = event.key.code - 26;
                    if (new_max_random_temp * 10 + digit <= max_temp) {
                        new_max_random_temp = new_max_random_temp * 10 + digit;
                        bottom_text.setString("Enter maximal particles temperature from " + std::to_string(new_min_random_temp) + " to 1275 K and press 'Enter'.  ---  Current chosen temperature: " + std::to_string(new_max_random_temp));
                    }
                }
                if (minimal_rand_temp_done && event.type == Event::KeyPressed && event.key.code == Keyboard::BackSpace && new_max_random_temp > 0) {
                    new_max_random_temp -= (new_max_random_temp % 10);
                    new_max_random_temp /= 10;
                    bottom_text.setString("Enter maximal particles temperature from " + std::to_string(new_min_random_temp) + " to 1275 K and press 'Enter'.  ---  Current chosen temperature: " + std::to_string(new_max_random_temp));
                }

                if (minimal_rand_temp_done && event.type == Event::KeyPressed && event.key.code == Keyboard::Enter && new_min_random_temp <= new_max_random_temp) {
                    for (int i = std::min(last_mouse_cursor_grid_index.x, mouse_click_pos_grid_index.x); i <= std::max(last_mouse_cursor_grid_index.x, mouse_click_pos_grid_index.x); i++)
                        for (int j = std::min(last_mouse_cursor_grid_index.y, mouse_click_pos_grid_index.y); j <= std::max(last_mouse_cursor_grid_index.y, mouse_click_pos_grid_index.y); j++)
                            grid[i][j].temp = (new_min_random_temp + rand() % (new_max_random_temp - new_min_random_temp + 1)) * temp_booster;

                    for (int i = std::min(last_mouse_cursor_grid_index.x, mouse_click_pos_grid_index.x); i <= std::max(last_mouse_cursor_grid_index.x, mouse_click_pos_grid_index.x); i++)
                        for (int j = std::min(last_mouse_cursor_grid_index.y, mouse_click_pos_grid_index.y); j <= std::max(last_mouse_cursor_grid_index.y, mouse_click_pos_grid_index.y); j++)
                            for (int k = 0; k < 4; k++)
                                whether_in_queue[i][j][k] = 0;

                    for (int i = std::min(last_mouse_cursor_grid_index.x, mouse_click_pos_grid_index.x); i <= std::max(last_mouse_cursor_grid_index.x, mouse_click_pos_grid_index.x); i++)
                        for (int j = std::min(last_mouse_cursor_grid_index.y, mouse_click_pos_grid_index.y); j <= std::max(last_mouse_cursor_grid_index.y, mouse_click_pos_grid_index.y); j++) {
                            grid[i][j].tile.setFillColor(update_color_with_temp(grid[i][j].temp));
                            main_render_texture.draw(grid[i][j].tile);
                            check_temp_with_neighbours(i, j);
                        }

                    editor_mode = 0;
                    random_temp_choice = 0;
                    text_display = 0;
                }
                if (!minimal_rand_temp_done && event.type == Event::KeyPressed && (event.key.code == Keyboard::Num1 || event.key.code == Keyboard::Num2 || event.key.code == Keyboard::Num3 || event.key.code == Keyboard::Num4
                    || event.key.code == Keyboard::Num5 || event.key.code == Keyboard::Num6 || event.key.code == Keyboard::Num7 || event.key.code == Keyboard::Num8 || event.key.code == Keyboard::Num9 || (event.key.code == Keyboard::Num0 && new_min_random_temp != 0))) {
                    digit = event.key.code - 26;
                    if (new_min_random_temp * 10 + digit <= max_temp) {
                        new_min_random_temp = new_min_random_temp * 10 + digit;
                        bottom_text.setString("Enter minimal particles temperature from 0 to 1275 K and press 'Enter'.  ---  Current chosen temperature: " + std::to_string(new_min_random_temp));
                    }
                }
                if (!minimal_rand_temp_done && event.type == Event::KeyPressed && event.key.code == Keyboard::BackSpace && new_min_random_temp > 0) {
                    new_min_random_temp -= (new_min_random_temp % 10);
                    new_min_random_temp /= 10;
                    bottom_text.setString("Enter minimal particles temperature from 0 to 1275 K and press 'Enter'.  ---  Current chosen temperature: " + std::to_string(new_min_random_temp));
                }
                if (!minimal_rand_temp_done && event.type == Event::KeyPressed && event.key.code == Keyboard::Enter) {
                    minimal_rand_temp_done = 1;
                    new_max_random_temp = new_min_random_temp;
                    bottom_text.setString("Enter maximal particles temperature from " + std::to_string(new_min_random_temp) + " to 1275 K and press 'Enter'.  ---  Current chosen temperature: " + std::to_string(new_max_random_temp));
                }

            }
        }



        if (editor_mode && !text_display) {
            if (Mouse::getPosition(window).x >= 0 && Mouse::getPosition(window).x < window_width && Mouse::getPosition(window).y >= 0 && Mouse::getPosition(window).y < only_particle_window_height)
                last_mouse_cursor_grid_index = { Mouse::getPosition(window).x / particle_size,Mouse::getPosition(window).y / particle_size };
            rect_for_particles_editing.setPosition(std::min(last_mouse_cursor_grid_index.x, mouse_click_pos_grid_index.x) * particle_size + particle_size / 2, std::min(last_mouse_cursor_grid_index.y, mouse_click_pos_grid_index.y) * particle_size + particle_size / 2);
            rect_for_particles_editing.setSize(Vector2f((std::max(last_mouse_cursor_grid_index.x, mouse_click_pos_grid_index.x) - std::min(last_mouse_cursor_grid_index.x, mouse_click_pos_grid_index.x)) * particle_size - 1, (std::max(last_mouse_cursor_grid_index.y, mouse_click_pos_grid_index.y) - std::min(last_mouse_cursor_grid_index.y, mouse_click_pos_grid_index.y)) * particle_size - 1));
        }

        if (start) {
            //std::cout << minimal_temp_diff << '\n';
            if (pqueue_for_next_move.empty()) {
                if (minimal_temp_diff > 3) {
                    minimal_temp_diff /= 2;
                    for (int i = 0; i < window_width / particle_size; i++)
                        for (int j = 0; j < only_particle_window_height / particle_size; j++)
                            for (int k = 0; k < 4; k++)
                                whether_in_queue[i][j][k] = 0;

                    for (int i = 0; i < window_width / particle_size; i++)
                        for (int j = 0; j < only_particle_window_height / particle_size; j++)
                            check_temp_with_neighbours(i, j);
                }
                else if (minimal_temp_diff == 3) {
                    minimal_temp_diff = 2;
                    for (int i = 0; i < window_width / particle_size; i++)
                        for (int j = 0; j < only_particle_window_height / particle_size; j++)
                            for (int k = 0; k < 4; k++)
                                whether_in_queue[i][j][k] = 0;

                    for (int i = 0; i < window_width / particle_size; i++)
                        for (int j = 0; j < only_particle_window_height / particle_size; j++)
                            check_temp_with_neighbours(i, j);
                }
                else
                    start = 0;
            }
            set_particles_to_change();

            std::cout <<"Particles to change: " << pqueue_for_next_move.size() << '\n';
        }

        

        main_render_texture.display();
        window.clear();
        window.draw(main_sprite);
        if (editor_mode) {
            window.draw(rect_for_particles_editing);
            if (text_display)
                window.draw(bottom_text);
        }
        window.display();

    }
}

