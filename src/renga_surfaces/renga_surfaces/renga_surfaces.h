#pragma once
#import <RengaCOMAPI.tlb>
#include <Renga/IPlugin.h>
#include <Renga/EventHandler.hpp>
#include <Renga/ActionEventHandler.hpp>
#include <memory>
#include <list>
#include <string>
class renga_surfaces : public plugins::IPlugin
{
public:
	renga_surfaces();
	~renga_surfaces();

	bool initialize(const wchar_t* pluginDir) override;
	void stop() override;

private:
	void addHandler(Renga::ActionEventHandler* pHandler);

private:
	typedef std::unique_ptr<Renga::ActionEventHandler> HandlerPtr;

private:
	std::list<HandlerPtr> m_handlerContainer;
};

EXPORT_PLUGIN(renga_surfaces);