#define CROW_MAIN

#include <vector>
#include <iostream>
#include <random>

#include "crow_all.h"

//DATA STRUCTURES

// Time related structures / functions (time is represented in UNIX Timestamps (second accuracy))
using time_pair = std::pair<int, int>;
using av_times = std::vector<time_pair>;

crow::json::wvalue times_to_json(av_times times)
{
    std::vector<crow::json::wvalue> val;
    for (auto& item : times)
    {
        crow::json::wvalue single_value;
        single_value["start"] = item.first;
        single_value["end"] = item.second;
        val.push_back(single_value);
    }
    return crow::json::wvalue(val);
}

av_times times_from_json(crow::json::rvalue json)
{
    std::vector<crow::json::rvalue> json_list(json.lo());
    av_times ret;
    for (auto& item : json_list)
    {
        ret.push_back(time_pair(int(item["start"]), int(item["end"])));
    }
    return ret;
}
//times

// A person can join an event and specify what times they'll be available to participate
struct person
{
    std::string name;
    av_times person_times;

    person()
    {
    }

    person(const crow::json::rvalue& json)
    {
        name = std::string(json["name"]);
        person_times = times_from_json(json["times"]);
    }

    crow::json::wvalue to_json()
    {
        crow::json::wvalue ret;
        ret["name"] = name;
        ret["times"] = times_to_json(person_times);
        return ret;
    }


}; //person

//An event, has people who joined, and times the event can take place
struct event
{
    std::vector<person> people;
    av_times event_times;

    event()
    {
    }

    event(const crow::json::rvalue& json)
    {
        for (auto& item : json["people"].lo())
        {
            people.emplace_back(person(item));
        }
        event_times = times_from_json(json["times"]);
    }

    crow::json::wvalue to_json()
    {
        crow::json::wvalue ret;

        std::vector<crow::json::wvalue> people_json;
        for (auto& item : people)
        {
            people_json.push_back(item.to_json());
        }
        ret["people"] = crow::json::wvalue(people_json);
        ret["times"] = times_to_json(event_times);
        return ret;
    }
}; //event

//END OF DATA STRUCTURES

//UTILITIES
std::string generate_token(int length)
{
    static std::random_device              rd;
    static std::mt19937_64                 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    std::stringstream toReturn;
    toReturn << std::hex;
    for (int i=0; i<length;i++)
    {
        toReturn << dis(gen);
    }

    return toReturn.str();
}
//END OF UTILITIES

int main()
{
    //just testing whether the json functions work properly
/*    std::string jstr = "{\"times\":[{\"end\":1632344399,\"start\":1632258000}],\"people\":[{\"name\":\"dudeA\",\"times\":[]},{\"times\":[],\"name\":\"dudetteA\"},{\"name\":\"dudeB\",\"times\":[]},{\"name\":\"dudetteB\",\"times\":[]}]}";

    std::vector<person> ppl(4);

    ppl[0].name = "dudeA";
    ppl[1].name = "dudetteA";
    ppl[2].name = "dudeB";
    ppl[3].name = "dudetteB";

    event x;//(crow::json::load(jstr));

    x.people = ppl;
    x.event_times = {{1632258000, 1632344399}};

    std::cout << x.to_json().dump() << std::endl;
*/

    crow::SimpleApp app;
//HTTP ROUTES

    //Create a new event
    CROW_ROUTE(app, "/events").methods(crow::HTTPMethod::POST)
    ([](const crow::request& req){

        //generate a name
        std::string event_name = generate_token(8);

        //Create an event with the given times
        crow::json::rvalue info(crow::json::load(req.body));
        av_times given_times = times_from_json(info["times"]);
        event ret;
        ret.event_times = given_times;

        //Write event to file
        std::ofstream event_file (std::string("events/" + event_name + ".json"), std::ios::out);
        event_file << ret.to_json().dump();
        event_file.close();

        //provide URL as response
        return crow::response(201, event_name);
    });

    //Access an existing event
    CROW_ROUTE(app, "/events/<string>")
    ([](crow::response& res, std::string event_name){

        //Get the file specified in the URL (return a 404 if the file doesn't open)
        std::ifstream event_file((std::string("events/" + event_name + ".json")), std::ios::in);
        if (!event_file)
        {
            res.code = 404;
            res.end();
            return;
        }

        //convert the file to string
        std::string event_data((std::istreambuf_iterator<char>(event_file)), (std::istreambuf_iterator<char>()));
        event_file.close();

        //return the string directly, since the JSON can only come from the backend, it can be sent immediately to save time
        res.body = event_data;
        res.end();
    });

    //Add a person to an event
    CROW_ROUTE(app, "/events/<string>/join").methods(crow::HTTPMethod::POST)
    ([](const crow::request& req, crow::response& res, std::string event_name){

        //Get the file specified in the URL (return a 404 if the file doesn't open)
        std::ifstream event_file((std::string("events/" + event_name + ".json")), std::ios::in);
        if (!event_file)
        {
            res.code = 404;
            res.end();
            return;
        }

        //convert the file to string
        std::string event_data((std::istreambuf_iterator<char>(event_file)), (std::istreambuf_iterator<char>()));
        event_file.close();


        //Get the person's name and times
        person person_to_add(crow::json::load(req.body));

        //Recreate the event object and add the person to it
        event x(crow::json::load(event_data));
        x.people.push_back(person_to_add);

        //Write event to file
        std::ofstream new_event_file (std::string("events/" + event_name + ".json"), std::ios::out | std::ios::trunc);
        new_event_file << x.to_json().dump();
        new_event_file.close();

        //Return the new event body
        res.body = x.to_json().dump();
        res.end();
    });

//END OF HTTP ROUTES
    app.multithreaded().port(18080).run();
    return 0;
}
