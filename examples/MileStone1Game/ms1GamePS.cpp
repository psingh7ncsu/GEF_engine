#include <engine/engine.hpp>
#include <string>

#include <cstdlib> //rand()
#include <iostream> 

#include <deque>
#include <algorithm>


const int windowWidth = 1440;
const int windowHeight = 900;

class Pipe {
    public:

    Pipe(engine::Scene& scene, float startPosition = windowWidth):
        m_scene(scene)
    {
        //Randomly decide size of top + toplip, bottom + bottomlip
        int randomVal = rand() % 8;
        auto pipeWidth = windowWidth/20.0;

        m_top = scene.createEntity();


        m_scene.transform(m_top).position = { startPosition, 0};
        m_scene.addShape(m_top, {.size = {pipeWidth, windowHeight/10 * randomVal}, .color = {0, 255, 0, 255}});
        m_scene.addCollider(m_top, {.size = scene.getShape(m_top)->size});

        m_bottom = scene.createEntity();
        m_scene.transform(m_bottom).position = { startPosition, windowHeight/10 * (randomVal + 2)};
        m_scene.addShape(m_bottom, {.size = {pipeWidth, windowHeight/10 * (10-randomVal)}, .color = {0, 255, 0, 255}});
        m_scene.addCollider(m_bottom, {.size = scene.getShape(m_bottom)->size});

    }
    Pipe(const Pipe&) = delete;
    Pipe& operator=(const Pipe&) = delete;


    ~Pipe()
    {
        m_scene.destroyEntity(m_top);
        m_scene.destroyEntity(m_bottom);
    }

    void movePipe(float timeStep)
    {
        m_scene.transform(m_top).position.x -= m_pipeSpeed * timeStep;
        m_scene.transform(m_bottom).position.x -= m_pipeSpeed * timeStep;
    }

    bool checkCollision(engine::PhysicsSystem& p, engine::EntityId e) const
    {
        return p.isCollision(m_scene, m_top, e) || p.isCollision(m_scene, m_bottom, e); 
    }

    float getPos() const
    {
        return m_scene.transform(m_top).position.x;
    }

    void setSpeed(float speed) {m_pipeSpeed = speed;}

    private:

        engine::Scene& m_scene;
        engine::EntityId m_top;
        engine::EntityId m_bottom;
        engine::EntityId m_topLip;
        engine::EntityId m_bottomLip;

        float m_pipeSpeed{100};
        
};


class PipeManager {
    public:
        PipeManager(engine::Scene& scene):
            m_scene(scene)
        {
            Start();
        }
        ~PipeManager(){}

        void Update(float timeStep)
        {
            for (auto& pipe: m_pipes)
            {
                pipe.movePipe(timeStep);
            }

            if(m_pipes.size() > 0 && m_pipes.front().getPos() < 0)
            {
                m_pipes.pop_front();
                m_pipeSpeed += 10;
                m_pipes.emplace_back(m_scene, m_pipes.back().getPos() + distanceBetweenPipes);
                score++;

                for (auto& pipe: m_pipes)
                {
                    pipe.setSpeed(m_pipeSpeed);
                }
            }

        }

        void Start()
        {
            m_pipes.emplace_back(m_scene);
            m_pipes.emplace_back(m_scene, windowWidth + distanceBetweenPipes);
            m_pipes.emplace_back(m_scene, windowWidth + 2*distanceBetweenPipes);
            m_pipes.emplace_back(m_scene, windowWidth + 3*distanceBetweenPipes);
            m_pipes.emplace_back(m_scene, windowWidth + 4*distanceBetweenPipes);
            score = 0;
        }

        void Clear()
        {
            m_pipes.clear();
            m_pipeSpeed = 100;
        }

        int getScore() const {return score;}

        bool checkCollision(engine::PhysicsSystem& p, engine::EntityId e) const
        {
            for (auto& pipe: m_pipes)
            {
                if(pipe.checkCollision(p, e))
                {
                    return true;
                }
            }
            return false;
        }

    private:
        engine::Scene& m_scene;
        std::deque<Pipe> m_pipes;

        const int distanceBetweenPipes {600};

        int score {0};

        float m_pipeSpeed{100};
        
};


int jumpHelper(engine::InputHandler& input, engine::SC::SDL_Scancode key, engine::Scene& scene, 
    engine::EntityId e, float jumpHeight, float timeStep)
{
        if (input.isKeyPressed(key)) {
            scene.transform(e).position.y -= jumpHeight * timeStep;
            scene.getRigidBody(e)->velocity = {};
        }
        return 0;
}



int main()
{


    engine::log::init();
    engine::log::info("Milestone 1 game - psingh7");

    engine::Window window("Milestone 1 game - psingh7", windowWidth, windowHeight);
    engine::Renderer renderer(window);
    renderer.toggleScalingMode();
    auto input = engine::InputHandler();
    engine::Scene scene;



    const engine::EntityId PlayerCharacter = scene.createEntity();
    scene.transform(PlayerCharacter).position = { 100.0, windowHeight/2};
    scene.addShape(PlayerCharacter, {.size = {32.f, 32.f}, .color = {255, 0, 0, 255}});
    scene.addCollider(PlayerCharacter, {.size = scene.getShape(PlayerCharacter)->size});
    scene.addRigidBody(PlayerCharacter);


    const engine::EntityId ceiling = scene.createEntity();
    scene.transform(ceiling).position = { 100.0, windowHeight/10};
    scene.addShape(ceiling, {.size = {32.f, 32.f}, .color = {0, 255, 0, 255}});
    scene.addCollider(ceiling, {.size = scene.getShape(ceiling)->size});


    const engine::EntityId floor = scene.createEntity();
    scene.transform(floor).position = { 100.0, windowHeight - windowHeight/10 - 10};
    scene.addShape(floor, {.size = {32.f, 32.f}, .color = {0, 255, 0, 255}});
    scene.addCollider(floor, {.size = scene.getShape(floor)->size});

    
    PipeManager pipe1(scene);


    engine::PhysicsSystem physics(98.f);
    engine::Clock clock;

    float totalElapsed = 0.f;

    bool quitGame = false;
    bool scalingKeyWasPressed = false;

    while (!window.shouldClose() && !quitGame) {
        window.pollEvents();
        clock.tick();   
        float deltaSeconds = std::min(
            clock.deltaSeconds(),
            1.0f / 60.0f
        );

        physics.step(scene, deltaSeconds);

        jumpHelper(input, engine::SC::SDL_SCANCODE_SPACE, scene, PlayerCharacter, 600.0, deltaSeconds);

        //prevents falling below floor
        if (physics.isCollision(scene, PlayerCharacter, floor))
        {
            scene.transform(PlayerCharacter).position = { 100.0, scene.transform(floor).position.y - scene.getShape(floor)->size.y};
        }

        //prevents going above the ceiling
        if (physics.isCollision(scene, PlayerCharacter, ceiling))
        {
            scene.transform(PlayerCharacter).position = { 100.0, scene.transform(ceiling).position.y + scene.getShape(ceiling)->size.y};
        }

        pipe1.Update(deltaSeconds);

        if(pipe1.checkCollision(physics, PlayerCharacter))
        {
            pipe1.Clear();
            std::cout << "SCORE: " << pipe1.getScore() << std::endl;
        }

        //Restart Game
        if (input.isKeyPressed(engine::SC::SDL_SCANCODE_R)) {
            pipe1.Clear();
            pipe1.Start();
        }

        //Toggle Scaling mode 
        const bool scalingKeyIsPressed = input.isKeyPressed(engine::SC::SDL_SCANCODE_P);
        if (scalingKeyIsPressed && !scalingKeyWasPressed) {
            renderer.toggleScalingMode();
        }
        scalingKeyWasPressed = scalingKeyIsPressed;


        //Quit Game
        if (input.isKeyPressed(engine::SC::SDL_SCANCODE_ESCAPE)) {
            quitGame = true;
        }


        engine::advanceAnimations(scene, deltaSeconds);

        renderer.clear({0, 0, 255, 255});
        renderer.drawEntities(scene);
        renderer.present();
    }

    scene.destroyEntity(PlayerCharacter);
    engine::log::debug("PlayerCharacter destroyed, exists = {}", scene.hasEntity(PlayerCharacter));

    engine::log::info("Milestone 1 game - psingh7 exiting cleanly");
    return 0;


}