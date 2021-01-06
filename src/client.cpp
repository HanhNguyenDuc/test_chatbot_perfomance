#include <iostream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <string>
#include <cstdio>
#include <chrono>
#include <array>
#include <memory>
#include <iostream>
#include <fstream>
#include <map>
#include <random>
#include "cxxopts.hpp"
#include "threadpool.hpp"
#include <ctime>
#include <iomanip>

class Entity
{
private:
    std::string name, text;

public:
    ~Entity()
    {
    }
    Entity()
    {
    }

    Entity(std::string name, std::string text)
    {
        this->name = name;
        this->text = text;
    }

    void setName(std::string &name)
    {
        this->name = name;
    }

    void setText(std::string &text)
    {
        this->text = text;
    }

    std::string getName()
    {
        return this->name;
    }

    std::string getText()
    {
        return this->text;
    }
};

int randomInRange(int m)
{
    std::mt19937_64 rng64(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<unsigned long long> dis;
    return dis(rng64) % m;
    // return 1;
}

class Intent
{
private:
    std::string name, text;
    std::vector<Entity *> listEntity;

public:
    ~Intent()
    {
        for (int i = 0; i < listEntity.size(); i++)
        {
            delete listEntity[i];
        }
    }
    Intent()
    {
    }
    Intent(std::string js, std::map<std::string, std::string> em)
    {
        nlohmann::json json = nlohmann::json::parse(js);
        for (auto &elem : json["entities"])
        {
            std::string en = elem;
            Entity *e = new Entity(elem, em[elem]);
            listEntity.push_back(e);
        }
    }

    Intent(nlohmann::json j, nlohmann::json em)
    {
        // std::cout << "entity map: " << em.dump() << std::endl;
        this->name = j["name"];
        auto v_size = j["text"].size();
        this->text = j["text"][randomInRange(v_size)];
        for (auto &elem : j["entities"])
        {
            std::string en = elem;
            // std::cout << "entity name: " << en << std::endl;
            auto ve_size = em[en].size();
            Entity *e = new Entity(elem, em[en][randomInRange(ve_size)]);
            listEntity.push_back(e);
        }
    }

    std::vector<Entity *> getEntities()
    {
        return this->listEntity;
    }

    void setName(std::string &name)
    {
        this->name = name;
    }

    void setText(std::string &text)
    {
        this->text = text;
    }

    std::string getName()
    {
        return this->name;
    }

    std::string getText()
    {
        return this->text;
    }

    std::string toText()
    {
        for (int i = 0; i < listEntity.size(); i++)
        {
            Entity *e = listEntity[i];
            std::size_t s_pattern = this->text.find("{" + e->getName() + "}");
            this->text = this->text.replace(s_pattern, e->getName().length() + 2, e->getText());
        }
        return this->text;
    }
};

nlohmann::json loadEntitiesFromFile(std::string filePath)
{
    std::ifstream ifs(filePath);
    auto j_content = nlohmann::json::parse(ifs);
    return j_content;
}

nlohmann::json loadIntentsFromFile(std::string filePath)
{
    std::ifstream ifs(filePath);
    auto j_content = nlohmann::json::parse(ifs);
    return j_content;
}

nlohmann::json loadActionMappingFromFile(std::string filePath)
{
    std::ifstream ifs(filePath);
    auto j_content = nlohmann::json::parse(ifs);
    return j_content;
}

nlohmann::json intent_with_slots;
nlohmann::json intent_without_slots;
nlohmann::json intent_but_no_intent;
nlohmann::json allDemoEntities;
nlohmann::json actionMapping;

class Conversation
{
private:
    bool is_received = true;
    std::string cached_i = "NULL";
    Intent *storedIntent;
    std::vector<Entity *> storedEntities;
    std::string conversation_id;

public:
    ~Conversation()
    {
        delete storedIntent;
        for (int i = 0; i < storedEntities.size(); i++)
        {
            delete storedEntities[i];
        }
    }
    void set_conversation_id(std::string conversation_id)
    {
        this->conversation_id = conversation_id;
    }

    std::string get_conversation_id()
    {
        return this->conversation_id;
    }

    Conversation()
    {
        this->storedIntent = NULL;
    }

    bool get_is_received()
    {
        return this->is_received;
    }

    bool set_is_received(bool is_received)
    {
        // std::cout << "is_received is setted to " << this->is_received << std::endl;
        this->is_received = is_received;
    }

    void re_init()
    {
        this->storedIntent = NULL;
        // std::cout << "Pass 148 " << std::endl;
        this->storedEntities.empty();
    }

    void next(nlohmann::json &allDemoIntents, nlohmann::json &allDemoEntities)
    {
        int idx = randomInRange(allDemoIntents.size());
        // delete this->storedIntent;
        Intent *i = new Intent(allDemoIntents[idx], allDemoEntities);
        this->storeIntent(i);
        // std::cout << this->storedIntent->getText() << std::endl;
    }

    void storeEntity(Entity *entity)
    {
        for (int i = 0; i < this->storedEntities.size(); i++)
        {
            Entity *e = this->storedEntities[i];
            if (e->getName() == entity->getName())
            {
                this->storedEntities[i] = entity;
                return;
            }
        }
        this->storedEntities.push_back(entity);
    }

    void storeIntent(Intent *intent)
    {
        if (this->storedIntent != NULL && this->storedIntent->getName() != "no_intent")
        {
            this->cached_i = this->storedIntent->getName();
        }
        this->storedIntent = intent;
        auto el = this->storedIntent->getEntities();
        for (auto e : el)
        {
            this->storeEntity(e);
        }
    }

    std::string get_current_intent_text()
    {
        return this->storedIntent->toText();
    }

    std::string toActionKey()
    {
        std::string res;
        if (this->storedIntent == NULL)
        {
            res = "Intent:NULL";
        }
        else
        {
            if (this->storedIntent->getName() == "no_intent")
            {
                res = "Intent:" + this->cached_i;
            }
            else
            {
                res = "Intent:" + this->storedIntent->getName();
            }
        }

        if (this->storedEntities.size() != 0)
        {
            res += "|Entities:[";
            for (auto e : this->storedEntities)
            {
                res += e->getName() + ",";
            }
            res.replace(res.length() - 1, 1, "]");
        }
        return res;
    }

    std::string predict(nlohmann::json actionMapping)
    {
        return actionMapping[this->toActionKey()];
    }
};

std::vector<Conversation> conv_v;
std::vector<std::mutex> conv_m_v;
std::mutex conv_vector_m, conv_ele_m, send_m, print_m, count_m, receive_m, fail_m, sent_m;
std::string ip, port;
int true_predict = 0, received_request = 0, fail_request = 0, sent_request = 0;

std::string getApiURI(const std::string &serverIP, const std::string &serverPort, const std::string &route)
{
    return "http://" + serverIP + ":" + serverPort + route;
}

std::string intToString(const int input)
{
    std::string result;
    int minus = 0;
    int m = input;
    if (m < 0)
        minus = 1;
    if (m == 0)
    {
        return "0";
    }
    if (minus == 1)
        m *= -1;
    while (m > 0)
    {
        char x = m % 10 + '0';
        result += x;
        m /= 10;
    }
    if (minus == 1)
        result += "-";
    int n = result.length();
    for (int i = 0; i < n / 2; i++)
    {
        std::swap(result[i], result[n - i - 1]);
    }
    return result;
}

std::string initConversation(const std::string &serverIP, const std::string &serverPort)
{
    std::string route = "/apis/init";
    std::string url = getApiURI(serverIP, serverPort, route);
    auto r = cpr::Post(cpr::Url{(url)}, cpr::VerifySsl{false});
    std::cout << r.text << std::endl;
    auto responseJson = nlohmann::json::parse(r.text);
    auto conv_id = responseJson["conversation_id"];
    // check type
    // std::cout << typeid(conv_id).name() << std::endl;
    // // std::cout << typename(conv_id) << std::endl;
    // if (typeid(conv_id).name() == "N8nlohmann10basic_jsonISt3mapSt6vectorNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEEblmdSaNS_14adl_serializerEEE"){
    //     std::cout << "reach here" << std::endl;
    //     return conv_id;
    // }
    // else{
    //     return intToString(conv_id);
    // }
    return intToString(responseJson["conversation_id"]);
}

std::string sendMessage(const std::string &serverIP, const std::string &serverPort)
{
    std::string route = "/apis/conversation/";
    std::string url = getApiURI(serverIP, serverPort, route);
    auto r = cpr::Post(cpr::Url{(url)}, cpr::VerifySsl{false});
    auto responseJson = nlohmann::json::parse(r.text);
    return responseJson["action"];
}

template <typename rep = std::chrono::seconds::rep,
          typename period = std::chrono::seconds::period>
void sleep(std::chrono::duration<rep, period> sec)
{
    using sleep_duration = std::chrono::duration<long double, std::nano>;

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    long double elapsed_time =
        std::chrono::duration_cast<sleep_duration>(end - start).count();

    long double sleep_time =
        std::chrono::duration_cast<sleep_duration>(sec).count();

    while (std::isgreater(sleep_time, elapsed_time))
    {
        end = std::chrono::steady_clock::now();
        elapsed_time = std::chrono::duration_cast<sleep_duration>(end - start).count();
    }
}

using namespace std::chrono_literals;

int send_message(Conversation &conv, nlohmann::json intent_with_slots, nlohmann::json action_mapping, std::string ip, std::string port)
{
    try
    {
        conv.set_is_received(false);
        conv.next(intent_with_slots, allDemoEntities);

        std::string gt_action = conv.predict(action_mapping);
        std::string route = "/apis/conversation";
        std::string url = getApiURI(ip, port, route);
        std::string body = "{\"conversation_id\": \"" + conv.get_conversation_id() + "\", \"message\": \"" + conv.get_current_intent_text() + "\"}";

        const std::lock_guard<std::mutex> lock_m_(sent_m);
        sent_request++;
        lock_m_.~lock_guard();
        cpr::Response *r = new cpr::Response;
        try
        {
            *r = cpr::Post(cpr::Url{(url)}, cpr::VerifySsl{false},
                           cpr::Body{body},
                           cpr::Header{{"content-type", "application/json"}, {"content-length", std::to_string(body.length())}});
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        // std::lock_guard<std::mutex> lock_(conv_ele_m);
        // lock_.~lock_guard();

        auto status_code = r->status_code;
        const std::lock_guard<std::mutex> lock_fail(fail_m);
        if (status_code != 200)
        {
            fail_request++;
        }
        lock_fail.~lock_guard();

        conv.set_is_received(true);

        const std::lock_guard<std::mutex> lock_rei(receive_m);
        received_request++;
        lock_rei.~lock_guard();

        nlohmann::json *responseJson = new nlohmann::json;
        
        *responseJson = nlohmann::json::parse(r->text);
        auto action = responseJson->operator[]("action");
        // std::cout << action << " - " << gt_action << std::endl;
        if (action == gt_action)
        {
            const std::lock_guard<std::mutex> lock_count(count_m);
            true_predict++;
            lock_count.~lock_guard();
        }
        // delete &responseJson;

        delete r;
        delete responseJson;
        // Process
    }
    catch (const std::exception &e)
    {
        // std::cerr << e.what() << '\n';
    }
    return 1;
}

int getNowTimeStamp()
{
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    return ms.count();
}

std::string pretifyTimeStampMiliseconds(int input)
{
    int hours = input / 3600000;
    input -= hours * 3600000;
    int minutes = input / 60000;
    input -= minutes * 60000;
    int seconds = input / 1000;
    input -= seconds * 1000;
    int miliseconds = input;
    std::string result = "";
    if (hours >= 1)
    {
        result = result + intToString(hours) + "h ";
    }
    if (minutes >= 1)
    {
        result = result + intToString(minutes) + "m ";
    }
    result = result + intToString(seconds) + "s ";
    result = result + intToString(miliseconds) + "ms";
    return result;
}

int main(int argc, char **argv)
{
    cxxopts::Options options("aif3_client", "AI Force NLP Challenge Benchmark and Judge tool");
    options.add_options()("i,ip", "Server IP Address", cxxopts::value<std::string>()->default_value("127.0.0.1"))("p,port", "Service Port, Default value is 3000", cxxopts::value<std::string>()->default_value("3000"))("w,worker", "Number of max worker for parallel requesting", cxxopts::value<int>()->default_value("10"))("d,data", "Path to data source", cxxopts::value<std::string>()->default_value("data"))("n,conversation_number", "number of conversation", cxxopts::value<int>()->default_value("50"));
    auto args = options.parse(argc, argv);

    try
    {
        // Get config
        ip = args["ip"].as<std::string>();
        port = args["port"].as<std::string>();
        std::cout << "Requesting URI: " << getApiURI(ip, port, "") << std::endl;
        auto num_worker = args["worker"].as<int>();
        std::cout << "Number of worker: " << num_worker << std::endl;
        auto conv_number = args["conversation_number"].as<int>();

        // auto i_path = args["intents"].as<std::string>();
        // auto e_path = args["entities"].as<std::string>();
        auto root_data_source = args["data"].as<std::string>();

        // Load data
        std::string i_path = root_data_source + "/all_intent.json";
        std::string iws_path = root_data_source + "/all_intent_with_slot.json";
        std::string ni_path = root_data_source + "/no_intent.json";
        std::string e_path = root_data_source + "/all_entities.json";
        std::string a_path = root_data_source + "/action_mapping.json";
        intent_with_slots = loadIntentsFromFile(iws_path);
        intent_without_slots = loadIntentsFromFile(i_path);
        intent_but_no_intent = loadIntentsFromFile(ni_path);
        allDemoEntities = loadEntitiesFromFile(e_path);
        actionMapping = loadActionMappingFromFile(a_path);

        // Prepare Stage
        std::cout << "Initializing Conversaion" << std::endl;

        // test_graph();
        // Init conversation
        for (int i = 0; i < conv_number; i++)
        {
            Conversation *t_conv = new Conversation();
            t_conv->re_init();
            // request to API to get conversation id
            auto conv_id = initConversation(ip, port);
            t_conv->set_conversation_id(conv_id);
            conv_v.emplace_back(*t_conv);
            // Create mutex for this conversation
        }

        std::cout << "Successfully Initialize Conversation" << std::endl;

        // Assign Threadpool
        ThreadPool pool(num_worker);
        std::vector<std::future<int>> results;
        int start_time = getNowTimeStamp();
        while (true)
        {
            std::cout << sent_request << " request has been sent! - "
                      << received_request << " request has been received! - "
                      << fail_request << " request fail!";
            std::cout << "\r" << std::flush;

            if (sent_request >= 2500)
            {
                break;
            }
            if (results.size() >= 2500)
            {
                break;
            }
            bool flag = false;
            // std::cout << "result size: " << results.size() << std::endl;
            for (int i = 0; i < conv_number; i++)
            {
                const std::lock_guard<std::mutex> lock(conv_vector_m);
                // std::cout << "Get is received conversation" << i << ": " << conv_v[i].get_is_received() << std::endl;
                // std::cout << conv_v[i].get_is_received() << std::endl;
                if (conv_v[i].get_is_received())
                {
                    conv_v[i].set_is_received(false);
                    results.emplace_back(
                        pool.enqueue([i] {
                            int intent_type = int(std::rand()) % 3;
                            if (intent_type == 0)
                            {
                                return send_message(conv_v[i], intent_with_slots, actionMapping, ip, port);
                            }
                            if (intent_type == 1)
                            {
                                return send_message(conv_v[i], intent_without_slots, actionMapping, ip, port);
                            }
                            if (intent_type == 2)
                            {
                                return send_message(conv_v[i], intent_but_no_intent, actionMapping, ip, port);
                            }
                        }));
                    // std::cout << "Conversation " << i << " has published a message" << std::endl;
                }

                // lock.~lock_guard();
            }
            // if (flag == false){
            //     std::cout << "cant_find conv" << std::endl;
            // }
        }
        while (received_request <= sent_request)
        {
            if (received_request == sent_request)
            {
                std::cout << sent_request << " request has been sent! - "
                          << received_request << " request has been received! - "
                          << fail_request << " request fail!";
                std::cout << std::endl;
                break;
            }
            else
            {
                std::cout << sent_request << " request has been sent! - "
                          << received_request << " request has been received! - "
                          << fail_request << " request fail!";
                std::cout << "\r" << std::flush;
            }
        }
        int end_time = getNowTimeStamp();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << std::fixed;
        std::cout << std::setprecision(2);
        std::cout << "Summary: " << std::endl;
        std::cout << "Total Request time: " << pretifyTimeStampMiliseconds(end_time - start_time) << std::endl;
        std::cout << "Average Request time: " << pretifyTimeStampMiliseconds(((float)(end_time - start_time) / 2500)) << std::endl;
        std::cout << "Total true Response: " << true_predict << " - Percentage: " << (float)true_predict / 2500 << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    // std::cout << "Total true predict: " << true_predict << std::endl;
    return 0;
}
