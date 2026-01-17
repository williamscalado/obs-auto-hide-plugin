#pragma once

#include <QString>
#include <functional>

// Interface comum para diferentes softwares de apresentação
class IPresentationClient {
public:
    virtual ~IPresentationClient() = default;
    
    // Conecta ao software de apresentação
    virtual void connect(const QString& url) = 0;
    
    // Desconecta e para monitoramento
    virtual void disconnect() = 0;
    
    // Retorna se está monitorando
    virtual bool is_connected() = 0;
    
    // Callback quando estado do versículo muda
    // true = versículo visível
    // false = versículo não visível (música, logo, etc)
    std::function<void(bool verse_visible)> on_verse_changed;

    // Callback para solicitar desativação total do plugin
    // (Ex: quando detecta tipo "MUSIC")
    std::function<void()> on_deactivation_requested;
};
