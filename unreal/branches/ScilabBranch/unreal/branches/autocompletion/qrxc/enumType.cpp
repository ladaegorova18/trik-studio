#include "enumType.h"
#include "nameNormalizer.h"
#include "../utils/outFile.h"

bool EnumType::init(QDomElement const &element, QString const &context)
{
	if (NonGraphicType::init(element, context)) {
		for (QDomElement valueElement = element.firstChildElement("value"); 
			!valueElement.isNull();
			valueElement = valueElement.nextSiblingElement("value"))
		{
			mValues.append(valueElement.text());
		}
		return true;
	} else
		return false;
}

Type* EnumType::clone() const
{
	EnumType *result = new EnumType();
	Type::copyFields(result);
	result->mValues = mValues;
	return result;
}

bool EnumType::generateEnumValues(utils::OutFile &out, bool isNotFirst)
{
	if (mValues.isEmpty())
		return false;
	generateOneCase(out, isNotFirst);

	out() << "\t\tresult";
	foreach (QString value, mValues)
		out() << " << \"" << value << "\"";
	out() << ";\n";	
	return true;
}

void EnumType::generateOneCase(utils::OutFile &out, bool isNotFirst) const
{
	//QString name = NameNormalizer::normalize(qualifiedName());
	
	if (!isNotFirst)
		out() << "\tif (name == \"" << name() << "\")\n";
	else
		out() << "\telse if (name == \"" << name() << "\")\n";
}


