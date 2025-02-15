#include "ion_session.h"
#include "logs.h"

void IonSession::wentOnline() { LOG_TRACE(); }

void IonSession::wentOffline() { LOG_TRACE(); }

void IonSession::didReceived(std::span<std::byte> buffer) { LOG_TRACE(); }
