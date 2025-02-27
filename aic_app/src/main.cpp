#include "aic_manager.h"
#include "crow.h"

void AddTestApi(crow::SimpleApp &app)
{
    // GET 请求，返回 "Hello, World!"
    app.route_dynamic("/hello").methods("GET"_method)([]() { return "Hello, World!"; });

    // GET 请求，带参数
    app.route_dynamic("/greet/<string>").methods("GET"_method)([](const crow::request &req, std::string name) {
        return "Hello, " + name + "!";
    });
    // GET请求，接收 JSON 数据并返回 JSON 数据
    app.route_dynamic("/json_return").methods("GET"_method)([](const crow::request &req) {
        // 创建一个新的 JSON 响应
        crow::json::wvalue response_json;
        response_json["greeting"] = "Hello";
        response_json["age"] = 18;
        response_json["status"] = "success";

        // 返回 JSON 响应
        return crow::response{response_json};
    });

    // POST 请求，接收 JSON 数据并返回
    app.route_dynamic("/json").methods("POST"_method)([](const crow::request &req) {
        auto json_data = crow::json::load(req.body);
        if (!json_data)
            return crow::response(400);

        std::string message = json_data["message"].s();
        return crow::response(200, "Received message: " + message);
    });
}
void AddAicApi(crow::SimpleApp &app)
{
    // 查询接口，GET 请求
    app.route_dynamic("/restapi").methods("GET"_method)([](const crow::request &req) {
        std::cout << "/restapi  url_params =" << req.url_params << std::endl;

        auto name = req.url_params.get("name");
        if (name)
        {
            if (std::string(name) == "C612_aiTag")
            {
                std::string json_sting = "";
                AicManager::GetInstance().DealTDFQuery(json_sting);
                crow::json::wvalue response_json = crow::json::load(json_sting);
                return crow::response(response_json);
                // return crow::response(200, "查询成功，参数 name=" + std::string(name));
            }
            else if (std::string(name) == "C612_jh")
            {
                std::string json_sting = "";
                AicManager::GetInstance().DealPDIQuery(json_sting);
                crow::json::wvalue response_json = crow::json::load(json_sting);
                return crow::response(response_json);
            }
            else
            {
                std::string json_sting = "查询失败，参数不支持 name=" + std::string(name);
                crow::json::wvalue response_json;
                response_json["error_msg"] = json_sting;
                return crow::response(400, response_json);
            }
        }
        else
        {
            return crow::response(400, "请求中缺少 'name' 参数");
        }
    });

    // 设置接口，POST 请求
    app.route_dynamic("/restapi/set").methods("POST"_method, "HEAD"_method)([](const crow::request &req) {
        std::cout << "/restapi/set  req.method =" << crow::method_name(req.method) << std::endl;
        std::cout << "/restapi/set  req.url_params =" << req.url_params << std::endl;

        auto name = req.url_params.get("name");
        if (name)
        {
            if (std::string(name) == "C612_aic_spm_review")
            {
                return crow::response(200, std::string(name) + " Received message");
            }
            else if (std::string(name) == "C612_aic_spm_result")
            {
                return crow::response(200, std::string(name) + " Received message");
            }
            else
            {
                return crow::response(400, "查询失败，参数不支持 name=" + std::string(name));
            }
        }
        else
        {
            return crow::response(400, "请求中缺少 'name' 参数");
        }
    });
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s  config.json\n", argv[0]);
        return -1;
    }
    std::string config_json = std::string(argv[1]);
    if (AicManager::GetInstance().Init(config_json))
    {
        std::cout << "AicManager init success!" << std::endl;
        return -1;
    }

    crow::SimpleApp app;
    app.loglevel(crow::LogLevel::Debug);

    // AddTestApi(app);
    AddAicApi(app);
    const auto port = AicManager::GetInstance().GetPort();
    app.port(port).multithreaded().run();

    return 0;
}
