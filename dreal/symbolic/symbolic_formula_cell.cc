#include "dreal/symbolic/symbolic_formula_cell.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>

#include "dreal/symbolic/hash.h"
#include "dreal/symbolic/symbolic_environment.h"
#include "dreal/symbolic/symbolic_expression.h"
#include "dreal/symbolic/symbolic_formula.h"
#include "dreal/symbolic/symbolic_variable.h"
#include "dreal/symbolic/symbolic_variables.h"

namespace dreal {
namespace drake {
namespace symbolic {

using std::equal;
using std::hash;
using std::lexicographical_compare;
using std::ostream;
using std::ostringstream;
using std::runtime_error;
using std::set;
using std::shared_ptr;
using std::static_pointer_cast;
using std::string;

FormulaCell::FormulaCell(const FormulaKind k, const size_t hash)
    : kind_{k}, hash_{hash_combine(hash, static_cast<size_t>(kind_))} {}

RelationalFormulaCell::RelationalFormulaCell(const FormulaKind k,
                                             const Expression& lhs,
                                             const Expression& rhs)
    : FormulaCell{k, hash_combine(lhs.get_hash(), rhs)},
      e_lhs_{lhs},
      e_rhs_{rhs} {}

Variables RelationalFormulaCell::GetFreeVariables() const {
  Variables ret{e_lhs_.GetVariables()};
  ret.insert(e_rhs_.GetVariables());
  return ret;
}

bool RelationalFormulaCell::EqualTo(const FormulaCell& f) const {
  // Formula::EqualTo guarantees the following assertion.
  assert(get_kind() == f.get_kind());
  const auto& rel_f = static_cast<const RelationalFormulaCell&>(f);
  return e_lhs_.EqualTo(rel_f.e_lhs_) && e_rhs_.EqualTo(rel_f.e_rhs_);
}

bool RelationalFormulaCell::Less(const FormulaCell& f) const {
  // Formula::Less guarantees the following assertion.
  assert(get_kind() == f.get_kind());
  const auto& rel_f = static_cast<const RelationalFormulaCell&>(f);
  if (e_lhs_.Less(rel_f.e_lhs_)) {
    return true;
  }
  if (rel_f.e_lhs_.Less(e_lhs_)) {
    return false;
  }
  return e_rhs_.Less(rel_f.e_rhs_);
}

NaryFormulaCell::NaryFormulaCell(const FormulaKind k,
                                 const set<Formula>& formulas)
    : FormulaCell{k, hash_value<set<Formula>>{}(formulas)},
      formulas_{formulas} {}

Variables NaryFormulaCell::GetFreeVariables() const {
  Variables ret{};
  for (const auto& f : formulas_) {
    ret.insert(f.GetFreeVariables());
  }
  return ret;
}

bool NaryFormulaCell::EqualTo(const FormulaCell& f) const {
  // Formula::EqualTo guarantees the following assertion.
  assert(get_kind() == f.get_kind());
  const auto& nary_f = static_cast<const NaryFormulaCell&>(f);
  return equal(
      formulas_.cbegin(), formulas_.cend(), nary_f.formulas_.cbegin(),
      nary_f.formulas_.cend(),
      [](const Formula& f1, const Formula& f2) { return f1.EqualTo(f2); });
}

bool NaryFormulaCell::Less(const FormulaCell& f) const {
  // Formula::Less guarantees the following assertion.
  assert(get_kind() == f.get_kind());
  const auto& nary_f = static_cast<const NaryFormulaCell&>(f);
  return lexicographical_compare(
      formulas_.cbegin(), formulas_.cend(), nary_f.formulas_.cbegin(),
      nary_f.formulas_.cend(),
      [](const Formula& f1, const Formula& f2) { return f1.Less(f2); });
}

ostream& NaryFormulaCell::DisplayWithOp(ostream& os, const string& op) const {
  const set<Formula>& formulas{get_operands()};
  auto it(formulas.cbegin());
  assert(formulas.size() > 1u);
  os << "(";
  os << *it;
  ++it;
  while (it != formulas.cend()) {
    os << " " << op << " " << *it;
    ++it;
  }
  os << ")";
  return os;
}

FormulaTrue::FormulaTrue()
    : FormulaCell{FormulaKind::True, hash<string>{}("True")} {}

Variables FormulaTrue::GetFreeVariables() const { return Variables{}; }

bool FormulaTrue::EqualTo(const FormulaCell& f) const {
  // Formula::EqualTo guarantees the following assertion.
  assert(get_kind() == f.get_kind());
  return true;  // There is only one instance of this kind.
}

bool FormulaTrue::Less(const FormulaCell& f) const {
  // Formula::Less guarantees the following assertion.
  assert(get_kind() == f.get_kind());
  // True < True ==> false
  return false;
}

bool FormulaTrue::Evaluate(const Environment&) const { return true; }

Formula FormulaTrue::Substitute(const ExpressionSubstitution&,
                                const FormulaSubstitution&) const {
  return Formula::True();
}

ostream& FormulaTrue::Display(ostream& os) const { return os << "True"; }

FormulaFalse::FormulaFalse()
    : FormulaCell{FormulaKind::False, hash<string>{}("False")} {}

Variables FormulaFalse::GetFreeVariables() const { return Variables{}; }

bool FormulaFalse::EqualTo(const FormulaCell& f) const {
  // Formula::EqualTo guarantees the following assertion.
  assert(get_kind() == f.get_kind());
  return true;  // There is only one instance of this kind.
}

bool FormulaFalse::Less(const FormulaCell& f) const {
  // Formula::Less guarantees the following assertion.
  assert(get_kind() == f.get_kind());
  // False < False ==> false
  return false;
}

bool FormulaFalse::Evaluate(const Environment&) const { return false; }

Formula FormulaFalse::Substitute(const ExpressionSubstitution&,
                                 const FormulaSubstitution&) const {
  return Formula::False();
}

ostream& FormulaFalse::Display(ostream& os) const { return os << "False"; }

FormulaVar::FormulaVar(const Variable& v)
    : FormulaCell{FormulaKind::Var, hash_value<Variable>{}(v)}, var_{v} {
  // Dummy symbolic variable (ID = 0) should not be used in constructing
  // symbolic formulas.
  assert(!var_.is_dummy());
  assert(var_.get_type() == Variable::Type::BOOLEAN);
}

Variables FormulaVar::GetFreeVariables() const { return Variables{var_}; }

bool FormulaVar::EqualTo(const FormulaCell& f) const {
  // Formula::EqualTo guarantees the following assertion.
  assert(get_kind() == f.get_kind());
  const FormulaVar& f_var{static_cast<const FormulaVar&>(f)};
  return var_.equal_to(f_var.var_);
}

bool FormulaVar::Less(const FormulaCell& f) const {
  // Formula::Less guarantees the following assertion.
  assert(get_kind() == f.get_kind());
  const FormulaVar& f_var{static_cast<const FormulaVar&>(f)};
  return var_.less(f_var.var_);
}

bool FormulaVar::Evaluate(const Environment& env) const {
  const Environment::const_iterator it{env.find(var_)};
  if (it != env.cend()) {
    return static_cast<bool>(it->second);
  } else {
    ostringstream oss;
    oss << "The following environment does not have an entry for the "
           "variable "
        << var_ << "\n";
    oss << env << "\n";
    throw runtime_error(oss.str());
  }
}

Formula FormulaVar::Substitute(const ExpressionSubstitution&,
                               const FormulaSubstitution& formula_subst) const {
  const auto it = formula_subst.find(var_);
  if (it != formula_subst.end()) {
    return it->second;
  }
  return Formula{var_};
}

ostream& FormulaVar::Display(ostream& os) const { return os << var_; }

const Variable& FormulaVar::get_variable() const { return var_; }

FormulaEq::FormulaEq(const Expression& e1, const Expression& e2)
    : RelationalFormulaCell{FormulaKind::Eq, e1, e2} {}

namespace {
// Helper function for ExpressionEq::Evaluate and ExpressionNeq::Evaluate.
//
// Checks if `e1.EvaluatePartial(env)` and `e2.EvaluatePartial(env)` are
// structurally equal.
bool CheckStructuralEqualityUptoPartialEvaluation(const Expression& e1,
                                                  const Expression& e2,
                                                  const Environment& env) {
  // Trivial case where env = ∅.
  if (env.empty()) {
    return e1.EqualTo(e2);
  }
  // Since `Expression::Evaluate` is faster than `Expression::EvaluatePartial`,
  // we use:
  //  - `Expression::Evaluate`        if if (vars(e₁) ∪ vars(e₂) ⊆ dom(env).
  //  - `Expression::EvaluatePartial` otherwise.
  const Variables vars{e1.GetVariables() + e2.GetVariables()};
  if (vars.size() <= env.size() && vars.IsSubsetOf(env.domain())) {
    return e1.Evaluate(env) == e2.Evaluate(env);
  } else {
    return e1.EvaluatePartial(env).EqualTo(e2.EvaluatePartial(env));
  }
}
}  // namespace

bool FormulaEq::Evaluate(const Environment& env) const {
  return CheckStructuralEqualityUptoPartialEvaluation(
      get_lhs_expression(), get_rhs_expression(), env);
}

Formula FormulaEq::Substitute(const ExpressionSubstitution& expr_subst,
                              const FormulaSubstitution& formula_subst) const {
  return get_lhs_expression().Substitute(expr_subst, formula_subst) ==
         get_rhs_expression().Substitute(expr_subst, formula_subst);
}

ostream& FormulaEq::Display(ostream& os) const {
  return os << "(" << get_lhs_expression() << " = " << get_rhs_expression()
            << ")";
}

FormulaNeq::FormulaNeq(const Expression& e1, const Expression& e2)
    : RelationalFormulaCell{FormulaKind::Neq, e1, e2} {}

bool FormulaNeq::Evaluate(const Environment& env) const {
  return !CheckStructuralEqualityUptoPartialEvaluation(
      get_lhs_expression(), get_rhs_expression(), env);
}

Formula FormulaNeq::Substitute(const ExpressionSubstitution& expr_subst,
                               const FormulaSubstitution& formula_subst) const {
  return get_lhs_expression().Substitute(expr_subst, formula_subst) !=
         get_rhs_expression().Substitute(expr_subst, formula_subst);
}

ostream& FormulaNeq::Display(ostream& os) const {
  return os << "(" << get_lhs_expression() << " != " << get_rhs_expression()
            << ")";
}

FormulaGt::FormulaGt(const Expression& e1, const Expression& e2)
    : RelationalFormulaCell{FormulaKind::Gt, e1, e2} {}

bool FormulaGt::Evaluate(const Environment& env) const {
  return get_lhs_expression().Evaluate(env) >
         get_rhs_expression().Evaluate(env);
}

Formula FormulaGt::Substitute(const ExpressionSubstitution& expr_subst,
                              const FormulaSubstitution& formula_subst) const {
  return get_lhs_expression().Substitute(expr_subst, formula_subst) >
         get_rhs_expression().Substitute(expr_subst, formula_subst);
}

ostream& FormulaGt::Display(ostream& os) const {
  return os << "(" << get_lhs_expression() << " > " << get_rhs_expression()
            << ")";
}

FormulaGeq::FormulaGeq(const Expression& e1, const Expression& e2)
    : RelationalFormulaCell{FormulaKind::Geq, e1, e2} {}

bool FormulaGeq::Evaluate(const Environment& env) const {
  return get_lhs_expression().Evaluate(env) >=
         get_rhs_expression().Evaluate(env);
}

Formula FormulaGeq::Substitute(const ExpressionSubstitution& expr_subst,
                               const FormulaSubstitution& formula_subst) const {
  return get_lhs_expression().Substitute(expr_subst, formula_subst) >=
         get_rhs_expression().Substitute(expr_subst, formula_subst);
}

ostream& FormulaGeq::Display(ostream& os) const {
  return os << "(" << get_lhs_expression() << " >= " << get_rhs_expression()
            << ")";
}

FormulaLt::FormulaLt(const Expression& e1, const Expression& e2)
    : RelationalFormulaCell{FormulaKind::Lt, e1, e2} {}

bool FormulaLt::Evaluate(const Environment& env) const {
  return get_lhs_expression().Evaluate(env) <
         get_rhs_expression().Evaluate(env);
}

Formula FormulaLt::Substitute(const ExpressionSubstitution& expr_subst,
                              const FormulaSubstitution& formula_subst) const {
  return get_lhs_expression().Substitute(expr_subst, formula_subst) <
         get_rhs_expression().Substitute(expr_subst, formula_subst);
}

ostream& FormulaLt::Display(ostream& os) const {
  return os << "(" << get_lhs_expression() << " < " << get_rhs_expression()
            << ")";
}

FormulaLeq::FormulaLeq(const Expression& e1, const Expression& e2)
    : RelationalFormulaCell{FormulaKind::Leq, e1, e2} {}

bool FormulaLeq::Evaluate(const Environment& env) const {
  return get_lhs_expression().Evaluate(env) <=
         get_rhs_expression().Evaluate(env);
}

Formula FormulaLeq::Substitute(const ExpressionSubstitution& expr_subst,
                               const FormulaSubstitution& formula_subst) const {
  return get_lhs_expression().Substitute(expr_subst, formula_subst) <=
         get_rhs_expression().Substitute(expr_subst, formula_subst);
}

ostream& FormulaLeq::Display(ostream& os) const {
  return os << "(" << get_lhs_expression() << " <= " << get_rhs_expression()
            << ")";
}

FormulaAnd::FormulaAnd(const set<Formula>& formulas)
    : NaryFormulaCell{FormulaKind::And, formulas} {
  assert(get_operands().size() > 1u);
}

FormulaAnd::FormulaAnd(const Formula& f1, const Formula& f2)
    : NaryFormulaCell{FormulaKind::And, set<Formula>{f1, f2}} {}

bool FormulaAnd::Evaluate(const Environment& env) const {
  for (const auto& f : get_operands()) {
    if (!f.Evaluate(env)) {
      return false;
    }
  }
  return true;
}

Formula FormulaAnd::Substitute(const ExpressionSubstitution& expr_subst,
                               const FormulaSubstitution& formula_subst) const {
  Formula ret{Formula::True()};
  for (const auto& f : get_operands()) {
    ret = ret && f.Substitute(expr_subst, formula_subst);
    // short-circuiting
    if (is_false(ret)) {
      return ret;
    }
  }
  return ret;
}

ostream& FormulaAnd::Display(ostream& os) const {
  return DisplayWithOp(os, "and");
}

FormulaOr::FormulaOr(const set<Formula>& formulas)
    : NaryFormulaCell{FormulaKind::Or, formulas} {
  assert(get_operands().size() > 1u);
}

FormulaOr::FormulaOr(const Formula& f1, const Formula& f2)
    : NaryFormulaCell{FormulaKind::Or, set<Formula>{f1, f2}} {}

bool FormulaOr::Evaluate(const Environment& env) const {
  for (const auto& f : get_operands()) {
    if (f.Evaluate(env)) {
      return true;
    }
  }
  return false;
}

Formula FormulaOr::Substitute(const ExpressionSubstitution& expr_subst,
                              const FormulaSubstitution& formula_subst) const {
  Formula ret{Formula::False()};
  for (const auto& f : get_operands()) {
    ret = ret || f.Substitute(expr_subst, formula_subst);
    // short-circuiting
    if (is_true(ret)) {
      return ret;
    }
  }
  return ret;
}

ostream& FormulaOr::Display(ostream& os) const {
  return DisplayWithOp(os, "or");
}

FormulaNot::FormulaNot(const Formula& f)
    : FormulaCell{FormulaKind::Not, f.get_hash()}, f_{f} {}

Variables FormulaNot::GetFreeVariables() const { return f_.GetFreeVariables(); }

bool FormulaNot::EqualTo(const FormulaCell& f) const {
  // Formula::EqualTo guarantees the following assertion.
  assert(get_kind() == f.get_kind());
  const FormulaNot& f_not{static_cast<const FormulaNot&>(f)};
  return f_.EqualTo(f_not.f_);
}

bool FormulaNot::Less(const FormulaCell& f) const {
  // Formula::Less guarantees the following assertion.
  assert(get_kind() == f.get_kind());
  const FormulaNot& not_f{static_cast<const FormulaNot&>(f)};
  return f_.Less(not_f.f_);
}

bool FormulaNot::Evaluate(const Environment& env) const {
  return !f_.Evaluate(env);
}

Formula FormulaNot::Substitute(const ExpressionSubstitution& expr_subst,
                               const FormulaSubstitution& formula_subst) const {
  return !f_.Substitute(expr_subst, formula_subst);
}

ostream& FormulaNot::Display(ostream& os) const {
  return os << "!(" << f_ << ")";
}

FormulaForall::FormulaForall(const Variables& vars, const Formula& f)
    : FormulaCell{FormulaKind::Forall, hash_combine(vars.get_hash(), f)},
      vars_{vars},
      f_{f} {}

Variables FormulaForall::GetFreeVariables() const {
  return f_.GetFreeVariables() - vars_;
}

bool FormulaForall::EqualTo(const FormulaCell& f) const {
  // Formula::EqualTo guarantees the following assertion.
  assert(get_kind() == f.get_kind());
  const FormulaForall& f_forall{static_cast<const FormulaForall&>(f)};
  return vars_ == f_forall.vars_ && f_.EqualTo(f_forall.f_);
}

bool FormulaForall::Less(const FormulaCell& f) const {
  // Formula::Less guarantees the following assertion.
  assert(get_kind() == f.get_kind());
  const FormulaForall& forall_f{static_cast<const FormulaForall&>(f)};
  if (vars_ < forall_f.vars_) {
    return true;
  }
  if (forall_f.vars_ < vars_) {
    return false;
  }
  return this->f_.Less(forall_f.f_);
}

bool FormulaForall::Evaluate(const Environment&) const {
  // Given ∀ x1, ..., xn. F, check if there is a counterexample satisfying
  // ¬F. If exists, it returns false. Otherwise, return true.
  // That is, it returns !check(∃ x1, ..., xn. ¬F)

  throw runtime_error("not implemented yet");
}

Formula FormulaForall::Substitute(
    const ExpressionSubstitution& expr_subst,
    const FormulaSubstitution& formula_subst) const {
  // Quantified variables are already bound and should not be substituted by s.
  // We construct a new substitution new_s from s by removing the entries of
  // bound variables.
  ExpressionSubstitution new_expr_subst{expr_subst};
  FormulaSubstitution new_formula_subst{formula_subst};
  for (const Variable& var : vars_) {
    if (var.get_type() == Variable::Type::BOOLEAN) {
      new_formula_subst.erase(var);
    } else {
      new_expr_subst.erase(var);
    }
  }
  return forall(vars_, f_.Substitute(new_expr_subst, new_formula_subst));
}

ostream& FormulaForall::Display(ostream& os) const {
  return os << "forall(" << vars_ << ". " << f_ << ")";
}

bool is_false(const FormulaCell& f) {
  return f.get_kind() == FormulaKind::False;
}

bool is_true(const FormulaCell& f) { return f.get_kind() == FormulaKind::True; }

bool is_variable(const FormulaCell& f) {
  return f.get_kind() == FormulaKind::Var;
}

bool is_equal_to(const FormulaCell& f) {
  return f.get_kind() == FormulaKind::Eq;
}

bool is_not_equal_to(const FormulaCell& f) {
  return f.get_kind() == FormulaKind::Neq;
}

bool is_greater_than(const FormulaCell& f) {
  return f.get_kind() == FormulaKind::Gt;
}

bool is_greater_than_or_equal_to(const FormulaCell& f) {
  return f.get_kind() == FormulaKind::Geq;
}

bool is_less_than(const FormulaCell& f) {
  return f.get_kind() == FormulaKind::Lt;
}

bool is_less_than_or_equal_to(const FormulaCell& f) {
  return f.get_kind() == FormulaKind::Leq;
}

bool is_relational(const FormulaCell& f) {
  return is_equal_to(f) || is_not_equal_to(f) || is_greater_than(f) ||
         is_greater_than_or_equal_to(f) || is_less_than(f) ||
         is_less_than_or_equal_to(f);
}

bool is_conjunction(const FormulaCell& f) {
  return f.get_kind() == FormulaKind::And;
}

bool is_disjunction(const FormulaCell& f) {
  return f.get_kind() == FormulaKind::Or;
}

bool is_nary(const FormulaCell& f) {
  return is_conjunction(f) || is_disjunction(f);
}

bool is_negation(const FormulaCell& f) {
  return f.get_kind() == FormulaKind::Not;
}

bool is_forall(const FormulaCell& f) {
  return f.get_kind() == FormulaKind::Forall;
}

shared_ptr<const FormulaFalse> to_false(
    const shared_ptr<const FormulaCell>& f_ptr) {
  assert(is_false(*f_ptr));
  return static_pointer_cast<const FormulaFalse>(f_ptr);
}

shared_ptr<const FormulaFalse> to_false(const Formula& f) {
  return to_false(f.ptr_);
}

shared_ptr<const FormulaTrue> to_true(
    const shared_ptr<const FormulaCell>& f_ptr) {
  assert(is_true(*f_ptr));
  return static_pointer_cast<const FormulaTrue>(f_ptr);
}

shared_ptr<const FormulaTrue> to_true(const Formula& f) {
  return to_true(f.ptr_);
}

shared_ptr<const FormulaVar> to_variable(
    const shared_ptr<const FormulaCell>& f_ptr) {
  assert(is_variable(*f_ptr));
  return static_pointer_cast<const FormulaVar>(f_ptr);
}

shared_ptr<const FormulaVar> to_variable(const Formula& f) {
  return to_variable(f.ptr_);
}

shared_ptr<const RelationalFormulaCell> to_relational(
    const shared_ptr<const FormulaCell>& f_ptr) {
  assert(is_relational(*f_ptr));
  return static_pointer_cast<const RelationalFormulaCell>(f_ptr);
}

shared_ptr<const RelationalFormulaCell> to_relational(const Formula& f) {
  return to_relational(f.ptr_);
}

shared_ptr<const FormulaEq> to_equal_to(
    const shared_ptr<const FormulaCell>& f_ptr) {
  assert(is_equal_to(*f_ptr));
  return static_pointer_cast<const FormulaEq>(f_ptr);
}

shared_ptr<const FormulaEq> to_equal_to(const Formula& f) {
  return to_equal_to(f.ptr_);
}

shared_ptr<const FormulaNeq> to_not_equal_to(
    const shared_ptr<const FormulaCell>& f_ptr) {
  assert(is_not_equal_to(*f_ptr));
  return static_pointer_cast<const FormulaNeq>(f_ptr);
}

shared_ptr<const FormulaNeq> to_not_equal_to(const Formula& f) {
  return to_not_equal_to(f.ptr_);
}

shared_ptr<const FormulaGt> to_greater_than(
    const shared_ptr<const FormulaCell>& f_ptr) {
  assert(is_greater_than(*f_ptr));
  return static_pointer_cast<const FormulaGt>(f_ptr);
}

shared_ptr<const FormulaGt> to_greater_than(const Formula& f) {
  return to_greater_than(f.ptr_);
}

shared_ptr<const FormulaGeq> to_greater_than_or_equal_to(
    const shared_ptr<const FormulaCell>& f_ptr) {
  assert(is_greater_than_or_equal_to(*f_ptr));
  return static_pointer_cast<const FormulaGeq>(f_ptr);
}

shared_ptr<const FormulaGeq> to_greater_than_or_equal_to(const Formula& f) {
  return to_greater_than_or_equal_to(f.ptr_);
}

shared_ptr<const FormulaLt> to_less_than(
    const shared_ptr<const FormulaCell>& f_ptr) {
  assert(is_less_than(*f_ptr));
  return static_pointer_cast<const FormulaLt>(f_ptr);
}

shared_ptr<const FormulaLt> to_less_than(const Formula& f) {
  return to_less_than(f.ptr_);
}

shared_ptr<const FormulaLeq> to_less_than_or_equal_to(
    const shared_ptr<const FormulaCell>& f_ptr) {
  assert(is_less_than_or_equal_to(*f_ptr));
  return static_pointer_cast<const FormulaLeq>(f_ptr);
}

shared_ptr<const FormulaLeq> to_less_than_or_equal_to(const Formula& f) {
  return to_less_than_or_equal_to(f.ptr_);
}

shared_ptr<const NaryFormulaCell> to_nary(
    const shared_ptr<const FormulaCell>& f_ptr) {
  assert(is_nary(*f_ptr));
  return static_pointer_cast<const NaryFormulaCell>(f_ptr);
}

shared_ptr<const NaryFormulaCell> to_nary(const Formula& f) {
  return to_nary(f.ptr_);
}

shared_ptr<const FormulaAnd> to_conjunction(
    const shared_ptr<const FormulaCell>& f_ptr) {
  assert(is_conjunction(*f_ptr));
  return static_pointer_cast<const FormulaAnd>(f_ptr);
}

shared_ptr<const FormulaAnd> to_conjunction(const Formula& f) {
  return to_conjunction(f.ptr_);
}

shared_ptr<const FormulaOr> to_disjunction(
    const shared_ptr<const FormulaCell>& f_ptr) {
  assert(is_disjunction(*f_ptr));
  return static_pointer_cast<const FormulaOr>(f_ptr);
}

shared_ptr<const FormulaOr> to_disjunction(const Formula& f) {
  return to_disjunction(f.ptr_);
}

shared_ptr<const FormulaNot> to_negation(
    const shared_ptr<const FormulaCell>& f_ptr) {
  assert(is_negation(*f_ptr));
  return static_pointer_cast<const FormulaNot>(f_ptr);
}

shared_ptr<const FormulaNot> to_negation(const Formula& f) {
  return to_negation(f.ptr_);
}

shared_ptr<const FormulaForall> to_forall(
    const shared_ptr<const FormulaCell>& f_ptr) {
  assert(is_forall(*f_ptr));
  return static_pointer_cast<const FormulaForall>(f_ptr);
}

shared_ptr<const FormulaForall> to_forall(const Formula& f) {
  return to_forall(f.ptr_);
}

}  // namespace symbolic
}  // namespace drake
}  // namespace dreal
