#pragma once

#include <generatorBase/generatorFactoryBase.h>

namespace trik {

/// Generator factory implemtation for TRIK platform
class TrikGeneratorFactory : public generatorBase::GeneratorFactoryBase
{
public:
	TrikGeneratorFactory(qrRepo::RepoApi const &repo
			, qReal::ErrorReporterInterface &errorReporter);

	virtual ~TrikGeneratorFactory();

	generatorBase::simple::AbstractSimpleGenerator *simpleGenerator(qReal::Id const &id
			, generatorBase::GeneratorCustomizer &customizer) override;

	QString pathToTemplates() const override;

	generatorBase::simple::Binding::MultiConverterInterface *enginesConverter() const override;

	generatorBase::simple::Binding::ConverterInterface *inputPortConverter() const override;

	generatorBase::simple::Binding::ConverterInterface *outputPortConverter() const override;

protected:
	void initVariables() override;
};

}
