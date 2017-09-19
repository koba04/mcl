#define CYBOZU_TEST_DISABLE_AUTO_RUN
#include <cybozu/test.hpp>
#include <cybozu/benchmark.hpp>
#include <cybozu/option.hpp>
#include <cybozu/xorshift.hpp>
#include <mcl/bn512.hpp>
#include <mcl/bn.hpp>

using namespace mcl::bn512;

mcl::fp::Mode g_mode;

void testCurve(const mcl::bn::CurveParam& cp)
{
	initPairing(cp, g_mode);
	G1 P;
	G2 Q;
	BN::mapToG1(P, 1);
	BN::mapToG2(Q, 1);
	GT e1, e2;
	BN::pairing(e1, P, Q);
	cybozu::XorShift rg;
	mpz_class a, b;
	Fr r;
	r.setRand(rg); a = r.getMpz();
	r.setRand(rg); b = r.getMpz();
	G1 aP;
	G2 bQ;
	G1::mul(aP, P, a);
	G2::mul(bQ, Q, b);
	BN::pairing(e2, aP, bQ);
	GT::pow(e1, e1, a * b);
	CYBOZU_TEST_EQUAL(e1, e2);
	CYBOZU_BENCH_C("G1::mulCT", 500, G1::mul, aP, aP, a);
	CYBOZU_BENCH_C("G1::add", 500, G1::add, aP, aP, P);
	CYBOZU_BENCH_C("G1::dbl", 500, G1::dbl, aP, aP);
	CYBOZU_BENCH_C("G2::mulCT", 500, G2::mul, bQ, bQ, b);
	CYBOZU_BENCH_C("G2::add", 500, G2::add, bQ, bQ, Q);
	CYBOZU_BENCH_C("G2::dbl", 500, G2::dbl, bQ, bQ);
	CYBOZU_BENCH("pairing", BN::pairing, e1, P, Q);
	CYBOZU_BENCH("finalExp", BN::finalExp, e1, e1);
}

CYBOZU_TEST_AUTO(pairing)
{
	puts("CurveFp462");
	testCurve(mcl::bn::CurveFp462);
	puts("CurveFp382_1");
	testCurve(mcl::bn::CurveFp382_1);
	puts("CurveFp382_2");
	testCurve(mcl::bn::CurveFp382_2);
	puts("CurveFp254BNb");
	testCurve(mcl::bn::CurveFp254BNb);
}

int main(int argc, char *argv[])
	try
{
	cybozu::Option opt;
	std::string mode;
	opt.appendOpt(&mode, "auto", "m", ": mode(gmp/gmp_mont/llvm/llvm_mont/xbyak)");
	if (!opt.parse(argc, argv)) {
		opt.usage();
		return 1;
	}
	g_mode = mcl::fp::StrToMode(mode);
	return cybozu::test::autoRun.run(argc, argv);
} catch (std::exception& e) {
	printf("ERR %s\n", e.what());
	return 1;
}