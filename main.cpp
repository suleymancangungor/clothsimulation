#include <SFML/Graphics.hpp>
#include <math.h>
#include <vector>
#include <iostream>

using namespace std;

const int WIDTH = 800;
const int HEIGHT = 600;
const float delta_time = 0.016f;

class Particle{

public :
    float posX;
    float posY;
    float prev_posX;
    float prev_posY;
    float accX = 0.0f;
    float accY = 3.0f;
    bool locked = false;

    Particle(float posX, float posY) {
        this->posX = posX;
        this->posY = posY;
        this->prev_posX = posX;
        this->prev_posY = posY;
    }

    void verletIntegration(float delta_time){
        float new_posX = posX *2 - prev_posX + accX * delta_time * delta_time ;
        float new_posY = posY *2 - prev_posY + accY * delta_time * delta_time ;

        prev_posX = posX;
        prev_posY = posY;

        posX = new_posX;
        posY = new_posY;
    }

    void applyFriction(float frictionCoefficient) {
        accX *= frictionCoefficient;
        if (accX <= 0.01f) {accX = 0.0f;}
    }

    void update(float delta_time){
        if (!locked){verletIntegration(delta_time); applyFriction(0.05f); keepInScreen();}
    }

    void keepInScreen() {
        if (posX < 0) posX = 0;
        if (posX > ::WIDTH) posX = ::WIDTH;
        if (posY < 0) posY = 0;
        //if (posY > ::HEIGHT) posY = ::HEIGHT;
    }
};

class Rope {

public :
    Particle *particle;
    Particle *next;
    float connection_lenght;
    bool is_broken = false;

    Rope(Particle *particle, Particle *next){
        this->particle = particle;
        this->next = next;
        connection_lenght = sqrt((particle->posX - next->posX) * (particle->posX - next->posX) + (particle->posY - next->posY) * (particle->posY - next->posY));
    }

    void setPosAccordingToTension(){

        float centerX = (particle->posX + next->posX)/2.0f;
        float centerY = (particle->posY + next->posY)/2.0f;

        float dx = (particle->posX - next->posX);
        float dy = (particle->posY - next->posY);
        float distance = sqrt(dx * dx + dy * dy);

        if (distance/connection_lenght >= 1.03f) {
            is_broken = true;
            return;
        }

        if (distance> 0){
            dx = dx / distance;
            dy = dy / distance;
        }

        if (!particle->locked){
            particle->posX = centerX + (dx * connection_lenght/2.0);
            particle->posY = centerY + (dy * connection_lenght/2.0);
        }

        if (!next->locked){
            next->posX = centerX - (dx * connection_lenght/2.0);
            next->posY = centerY - (dy * connection_lenght/2.0);
        }
    }
};

vector<Particle*> particles;
vector<Rope*> ropes;

void update() {
    for (int i = 0; i<particles.size(); i++){
        particles[i]->update(delta_time);
    }

    for (int j=0 ; j<10; j++){
        for (int i =0 ; i<ropes.size(); i++) {
            if (!(ropes[i]->is_broken)) {ropes[i]->setPosAccordingToTension();}
        }
    }

}

 void render(sf::RenderWindow& window){

    window.clear();

    for (int i = 0; i < particles.size(); i++) {
        sf::Vertex point(sf::Vector2f(particles[i]->posX, particles[i]->posY), sf::Color::Red);
        window.draw(&point, 1, sf::Points); // Draw as a point
    }

    for (int i=0 ; i < ropes.size(); i++){
        if (!(ropes[i]->is_broken)) {
            Particle* p1 = ropes[i]->particle;
            Particle* p2 = ropes[i]->next;

            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(p1->posX, p1->posY), sf::Color::White),
                sf::Vertex(sf::Vector2f(p2->posX, p2->posY), sf::Color::White)
            };
            window.draw(line, 2, sf::PrimitiveType::Lines);
        }
    }

}

int main(){
    int rows = 50;
    int cols = 70;

    for (int j=0; j<rows; j++) {
        for(int i=0 ; i<cols; i++){
            float x = 50.0f + i*10.0f +((j==69) ? 0.0f : 5.0f);
            float y = 50.0f + j*10.0f;
            Particle *particle = new Particle(x,y);
            if (j==0){
                particle->locked = true;
            }
            particles.push_back(particle);
            if (i!=0){
                Rope *rope = new Rope(particles[j*cols+(i-1)],particle);
                ropes.push_back(rope);
            }
            if (j!=0) {
                Rope *rope = new Rope(particles[(j-1)*cols+i],particle);
                ropes.push_back(rope);
            }
        }
    }

    sf::RenderWindow window(sf::VideoMode(WIDTH,HEIGHT),"Cloth simulation");

    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window : exit
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            sf::Vector2i position = sf::Mouse::getPosition(window);
            int size = ropes.size();
            for (int i = size-1; i>=0; i--){
                Rope *rope = ropes[i];
                float x1 = rope->particle->posX;
                float y1 = rope->particle->posY;

                float x2 = rope->next->posX;
                float y2 = rope->next->posY;

                if (x1 > x2) {
                    int temp = x1;
                    x1 = x2;
                    x2 = temp;
                }

                if (y1 > y2) {
                    int temp = y1;
                    y1 = y2;
                    y2 = temp;
                }

                if (position.x <= x2 && position.x >= x1 && position.y <= y2 && position.y >= y1 && !(rope->is_broken)) {
                    ropes.erase( ropes.begin() + i );
                }
            }
        }

        update();
        render(window);

        // Update the window
        window.display();
        //cout << particles[1].posX << " " << particles[1].posY << " " << ropes[1].particle.posX << " " << ropes[1].particle.posY << endl;
    }

    return EXIT_SUCCESS;
}
