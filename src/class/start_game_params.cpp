#include "classes.hpp"
#include "app_state.hpp"


JNI::jclass start_game_params_class = nullptr;

static JNI::jlong current_place_id = -1;
static JNI::jlong current_user_id = -1;
static JNI::jint current_join_request_type = -1;
static JNI::jstring current_join_attempt_id = nullptr;
static JNI::jstring current_join_attempt_origin = nullptr;
static JNI::jstring current_referral_page = nullptr;

void create_start_game_params_class(JNI::JavaVM *vm) {
    start_game_params_class = JNI::create_class(vm, "com/roblox/engine/jni/autovalue/StartGameParams");
    start_game_params_class->method("accessCode", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return JNI::create_string("");
    });
    start_game_params_class->method("callId", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return JNI::create_string("");
    });
    start_game_params_class->method("conversationId", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return static_cast<JNI::jlong>(-1);
    });
    start_game_params_class->method("deviceParams", +[](JNI::Object *, va_list) -> JNI::ValueVariant {
        return get_device_params();
    });
    start_game_params_class->method("gameId", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return JNI::create_string("");
    });
    start_game_params_class->method("isUnder13", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return static_cast<JNI::jboolean>(user_state.under_13);
    });
    start_game_params_class->method("isoContext", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return JNI::create_string("");
    });
    start_game_params_class->method("joinAttemptId", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return current_join_attempt_id;
    });
    start_game_params_class->method("joinAttemptOrigin", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return current_join_attempt_origin;
    });
    start_game_params_class->method("joinRequestType", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return current_join_request_type;
    });
    start_game_params_class->method("launchData", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return JNI::create_string("");
    });
    start_game_params_class->method("linkCode", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return JNI::create_string("");
    });
    start_game_params_class->method("placeId", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return current_place_id;
    });
    start_game_params_class->method("platformParams", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return get_platform_params();
    });
    start_game_params_class->method("referralPage", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return current_referral_page;
    });
    start_game_params_class->method("reservedServerAccessCode", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return JNI::create_string("");
    });
    start_game_params_class->method("surface", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return new JNI::Object();
    });
    start_game_params_class->method("userId", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return current_user_id;
    });
    start_game_params_class->method("username", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return JNI::create_string("");
    });
    start_game_params_class->method("vrContext", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return nullptr;
    });
}

void set_start_game_params(long place_id, long user_id, int join_request_type, const std::string &join_attempt_id, const std::string &join_attempt_origin, const std::string &referral_page) {
    current_place_id = place_id;
    current_user_id = user_id;
    current_join_request_type = join_request_type;
    current_join_attempt_id = JNI::create_string(join_attempt_id);
    current_join_attempt_origin = JNI::create_string(join_attempt_origin);
    current_referral_page = JNI::create_string(referral_page);
}
