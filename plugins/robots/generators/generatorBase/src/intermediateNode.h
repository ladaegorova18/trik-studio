#pragma once

#include <qrrepo/repoApi.h>
#include <qrkernel/ids.h>

#include <generatorBase/semanticTree/semanticNode.h>
#include <generatorBase/semanticTree/semanticTree.h>

namespace generatorBase {
namespace myUtils {

class IntermediateNode : public QObject {

	Q_OBJECT

public:

	enum Type {
		simple
		, block
		, ifThenCondition
		, ifThenElseCondition
		, switchCondition
		, infiniteloop
		, whileloop
		, breakNode
		, nodeWithBreaks
	};

	explicit IntermediateNode(QObject *parent);

	QString currentThread() const;
	void setCurrentThread(const QString &thread);

	virtual QList<IntermediateNode *> childrenNodes() const = 0;

	virtual Type type() const = 0;
	virtual qReal::Id firstId() const = 0;
	virtual bool analyzeBreak() = 0;
	bool hasBreakInside() const;

protected:
	bool mHasBreakInside;
	bool mBreakWasAnalyzed;
	QString mCurrentThread;
};

class SimpleNode : public IntermediateNode {

	Q_OBJECT

public:
	explicit SimpleNode(const qReal::Id &id, QObject *parent = nullptr);

	Type type() const;
	qReal::Id firstId() const;
	bool analyzeBreak();
	QList<IntermediateNode *> childrenNodes() const override;

	qReal::Id id() const;
private:
	const qReal::Id mId;
};

class BreakNode : public IntermediateNode {

	Q_OBJECT

public:
	explicit BreakNode(const qReal::Id &id, QObject *parent);

	Type type() const;
	qReal::Id firstId() const;
	bool analyzeBreak();
	QList<IntermediateNode *> childrenNodes() const override;

private:
	const qReal::Id mId;
};

class IfNode : public IntermediateNode {

	Q_OBJECT

public:
	explicit IfNode(IntermediateNode *condition, IntermediateNode *thenBranch, IntermediateNode *elseBranch, IntermediateNode *exit, QObject *parent);

	IntermediateNode *condition() const;
	IntermediateNode *thenBranch() const;
	IntermediateNode *elseBranch() const;
	IntermediateNode *exit() const;

	bool analyzeBreak();
	QList<IntermediateNode *> childrenNodes() const override;
	Type type() const;
	qReal::Id firstId() const;
private:
	IntermediateNode *mCondition;
	IntermediateNode *mThenBranch;
	IntermediateNode *mElseBranch;
	IntermediateNode *mExit;
	bool mIsIfThenForm;
};


class NodeWithBreaks : public IntermediateNode {

	Q_OBJECT

public:
	explicit NodeWithBreaks(IntermediateNode *condition, QList<IntermediateNode *> &exitBranches, QObject *parent);

	IntermediateNode *condition() const;
	QList<IntermediateNode *> exitBranches() const;
	QList<IntermediateNode *> restBranches() const;

	void setRestBranches(const QList<IntermediateNode *> &restBranches);
	bool analyzeBreak();
	QList<IntermediateNode *> childrenNodes() const override;
	Type type() const;
	qReal::Id firstId() const;
private:
	IntermediateNode *mCondition;
	QList<IntermediateNode *> mExitBranches;
	QList<IntermediateNode *> mRestBranches;
};

class SwitchNode : public IntermediateNode {

	Q_OBJECT

public:
	explicit SwitchNode(IntermediateNode *condition, const QList<IntermediateNode *> &branches, IntermediateNode *exit, QObject *parent);

	IntermediateNode *condition() const;
	QList<IntermediateNode *> branches() const;
	IntermediateNode *exit() const;

	bool analyzeBreak();
	QList<IntermediateNode *> childrenNodes() const override;
	Type type() const;
	qReal::Id firstId() const;
private:
	IntermediateNode *mCondition;
	const QList<IntermediateNode *> mBranches;
	IntermediateNode *mExit;
};


class BlockNode : public IntermediateNode {

	Q_OBJECT

public:
	explicit BlockNode(IntermediateNode *firstNode, IntermediateNode *secondNode, QObject *parent);

	IntermediateNode *firstNode() const;
	IntermediateNode *secondNode() const;

	bool analyzeBreak();
	QList<IntermediateNode *> childrenNodes() const override;
	Type type() const;
	qReal::Id firstId() const;
private:
	IntermediateNode *mFirstNode;
	IntermediateNode *mSecondNode;
};

class WhileNode : public IntermediateNode {

	Q_OBJECT

public:
	explicit WhileNode(IntermediateNode *headNode, IntermediateNode *bodyNode, IntermediateNode *exitNode, QObject *parent);

	IntermediateNode *headNode() const;
	IntermediateNode *bodyNode() const;
	IntermediateNode *exitNode() const;

	bool analyzeBreak();
	QList<IntermediateNode *> childrenNodes() const override;
	Type type() const;
	qReal::Id firstId() const;
private:
	IntermediateNode *mHeadNode;
	IntermediateNode *mBodyNode;
	IntermediateNode *mExitNode;
};


class SelfLoopNode : public IntermediateNode {

	Q_OBJECT

public:
	explicit SelfLoopNode(IntermediateNode *bodyNode, QObject *parent = nullptr);

	IntermediateNode *bodyNode() const;

	bool analyzeBreak();
	QList<IntermediateNode *> childrenNodes() const override;
	Type type() const;
	qReal::Id firstId() const;
private:
	IntermediateNode *mBodyNode;
};

}
}
