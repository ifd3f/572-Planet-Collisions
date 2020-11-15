//
// Created by astrid on 11/3/20.
//

#ifndef LAB3_WORLD_H
#define LAB3_WORLD_H


#include <vector>
#include <glm/glm.hpp>
#include <memory>
#include <set>
#include <unordered_set>
#include <unordered_map>


struct Particle;

enum ContactState {
    CONTACT_STATE_APPROACHING,
    CONTACT_STATE_STABLE,
    CONTACT_STATE_LEAVING
};

struct Contact {
    Contact(Particle *pParticle, Particle *pParticle1, glm::vec3 vec);

    Particle *a, *b;
    // Collision normal oriented from b to a
    glm::vec3 normal;
    unsigned long lifetime = 0;
    unsigned long stable_time = 0;
    ContactState state = CONTACT_STATE_APPROACHING;

    void deintersect() const;
    void solve_momentum();
    void set_state(ContactState state);
    bool operator==(Contact other) const;
};

// For sorting the contacts in deepest to shallowest order
struct ContactDepthComparator {
    bool operator()(Contact a, Contact b) {
        return glm::length(a.normal) > glm::length(b.normal);
    }
};

namespace std {
    template<>
    struct hash<Contact> {
        std::size_t operator()(const Contact &c) const {
            return ((std::size_t) c.a + 31) + (std::size_t) c.b;
        }
    };
}

struct ContactGroup;

struct GroupSearchData {
    // If this group is stable (i.e. internal gravitation wouldn't change anything)
    bool stable;

    // Particles that are touching.
    std::vector<Particle*> touching;
    std::unordered_set<Contact*> contacts;
};

struct Particle {
    // State
    glm::vec3 pos, vel;
    // Accumulator
    glm::vec3 impulse;
    // Force
    glm::vec3 gravity;
    std::unordered_map<Particle*, Contact*> contacts;
    ContactGroup *group;

    float mass, radius;
    virtual void integrate(float dt);
    virtual void apply_acc(glm::vec3 r, glm::vec3 da);
    bool is_touching(Particle *other, glm::vec3 *normal);
    void solve_contacts();

    void reset();

    // DFS for particles in this group
    GroupSearchData get_group_members(std::unordered_set<Particle *> unvisited);
};

// Represents a bunch of particles touching each other.
struct ContactGroup {
    std::unordered_set<Particle*> particles;
    std::unordered_set<Contact*> contacts;
    bool stable = false;
    explicit ContactGroup(std::vector<Particle *> vector);
    void calculate_params();
    void mark_unstable();
};

struct Body : public Particle {
    glm::vec3 ang_vel;

    // Store rotation in this matrix. Quaternion would technically be better but idk how to ¯\_(ツ)_/¯
    glm::mat4 rotation;
    void integrate(float dt) override;
    void apply_acc(glm::vec3 r, glm::vec3 da) override;
};

struct World {
    std::vector<Particle*> particles;
    // The set of contacts.
    std::vector<Contact> contacts;
    std::vector<ContactGroup> groups;
    unsigned long steps = 0;

    World();

    void reset();
    void deintersect_all();

    void find_intersections();
    void solve_intersections();
    void solve_contacts();
    void create_groups();
    void gravitate(float dt);
    void integrate(float dt);

    void step(float dt);
};


#endif //LAB3_WORLD_H
