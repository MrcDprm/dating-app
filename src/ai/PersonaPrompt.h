#pragma once

#include "core/Profile.h"

// Hazır profilin karakterini ve sohbet kurallarını anlatan sistem talimatını oluşturur
QString buildPersonaPrompt(const Profile &persona, const Profile &user);
