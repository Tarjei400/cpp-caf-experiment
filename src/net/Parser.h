//
// Created by Adrian Jutrowski on 12/10/2020.
//

#ifndef SERVER_PARSER_H
#define SERVER_PARSER_H


using ParserType = typed_actor<replies_to<ParseMessage, Buffer>::with<int>>;

class ParserActor : public ParserType::base {
public:
    ParserActor(actor_config& cfg) : ParserType::base(cfg) {
        // nop
    }

    behavior_type make_behavior() override {
        return make_typed();
    }

    ParserType::behavior_type make_typed() {
        return {
                [=](ParseMessage, Buffer data) {
                    cout << "Parsed message: " << std::string(std::begin(data), std::end(data)) << endl;
                    return 1;
                },
        };
    }
};


#endif //SERVER_PARSER_H
