// SFML framework by Ari Alexander Tannahill Thomson
// Includes:
//  basic fixed and variable updates
//  basic input abstraction for controller
//  AABB and SAT collision detection


#include <iostream>

#include <SFML/Graphics.hpp>




//void handleEvents(sf::RenderWindow *wn, Input *in)
//{
//    while (const std::optional event = wn->pollEvent())
//    {
//        if (event->is<sf::Event::Closed>()) //on window being closed
//        {
//            wn->close();
//        }
//
//        if (const auto* resized = event->getIf<sf::Event::Resized>()) //on window size being changed
//        {
//            sf::FloatRect visible_area(sf::Vector2f(0, 0), sf::Vector2f(resized->size.x, resized->size.y));
//            wn->setView(sf::View(visible_area));
//        }
//        
//        if (const auto* joystick_button_pressed = event->getIf<sf::Event::JoystickButtonPressed>()) //gamepad buttons being pressed
//        {
//            in->setControllerButtonDown(joystick_button_pressed->button, true);
//        }
//
//        if (const auto* joystick_button_released= event->getIf<sf::Event::JoystickButtonReleased>()) //gamepad buttons being released
//        {
//            in->setControllerButtonDown(joystick_button_released->button, false);
//        }
//
//        if (const auto* joystickMoved = event->getIf<sf::Event::JoystickMoved>()) //leftstick or rightstick being moved
//        {
//            in->setJoystickAxis(joystickMoved->axis, joystickMoved->position);
//        }
//
//        if (const auto* keyboard_key_pressed = event->getIf<sf::Event::KeyPressed>())
//        {
//            in->setKeyDown(keyboard_key_pressed->code, true);
//        }
//
//        if (const auto* keyboard_key_released = event->getIf<sf::Event::KeyReleased>())
//        {
//            in->setKeyDown(keyboard_key_released->code, false);
//        }
//
//        if (const auto* mouse_button_pressed = event->getIf<sf::Event::MouseButtonPressed>())
//        {
//            in->setMouseButtonDown(mouse_button_pressed->button, true);
//        }
//
//        if (const auto* mouse_button_released = event->getIf<sf::Event::MouseButtonReleased>())
//        {
//            in->setMouseButtonDown(mouse_button_released->button, false);
//        }
//
//        if (const auto* mouse_moved = event->getIf<sf::Event::MouseMoved>()) // whenever the mouse moves this is called
//        {
//            in->setMousePosition(wn);
//        }
//    }
//
//}

//int main()
//{
//
//    sf::RenderWindow window(sf::VideoMode({ 720, 720 }), "SFML Framework");
//    
//    // disables repeat KeyPressed events, you can remove this line if you want this functionality
//    window.setKeyRepeatEnabled(false);
//
//    Game this_game(&window);
//    
//    Input input;
//    AudioManager& aud = AudioManager::getInstance();
// 
//     // Variable timestep, from https://gafferongames.com/post/fix_your_timestep/ accessed: 09/05/2025
//    // Glenn Fiedler, 2004
//
//
//    //maybe look at this later for adding frame interpolation https://en.sfml-dev.org/forums/index.php?topic=14483.0
//
//    sf::Clock clock;
//
//    const float dt = 1.f/60.f; //the dt we go for, this is for 60fps
//
//    float accumulator = 0;
//
//    float frame_time = 0;
//
//    while (window.isOpen())
//    {
//        handleEvents(&window, &input);
//
//        this_game.handleInput(&input);
//
//
//        sf::Time elapsed = clock.restart();
//
//        frame_time = elapsed.asSeconds();
//
//
//        accumulator += frame_time;
//
//        while (accumulator >= dt) //variable number of updates per frame to keep it as consistent as possible for physics etc
//        {
//
//            this_game.fixedUpdate(dt);
//
//            this_game.handleCollisions(dt);
//
//            accumulator -= dt;
//        }
//
//        this_game.variableUpdate(frame_time); //happens once perframe to allow for better timing
//      
//        //later add an interpolation, maybe get a list of all enitites and use their velocity
//        this_game.renderGame();
//
//        // reset the pressed/released functions for next frame
//        input.update();
//    }
//
//    return 0;
//}

