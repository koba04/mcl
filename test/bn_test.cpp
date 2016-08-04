#define PUT(x) std::cout << #x "=" << x << std::endl;
#define CYBOZU_TEST_DISABLE_AUTO_RUN
#include <cybozu/benchmark.hpp>
//cybozu::CpuClock clk;
#include <cybozu/test.hpp>
#include <mcl/bn.hpp>
#include <cybozu/option.hpp>

typedef mcl::FpT<mcl::FpTag, 256> Fp;
typedef mcl::bn::BNT<Fp> BN;
typedef BN::Fp2 Fp2;
typedef BN::Fp6 Fp6;
typedef BN::Fp12 Fp12;
typedef BN::G1 G1;
typedef BN::G2 G2;

mcl::fp::Mode g_mode;

const struct TestSet {
	mcl::bn::CurveParam cp;
	struct G2 {
		const char *aa;
		const char *ab;
		const char *ba;
		const char *bb;
	} g2;
	struct G1 {
		int a;
		int b;
	} g1;
	const char *e;
} g_testSetTbl[] = {
	{
		mcl::bn::CurveFp254BNb,
		{
			"12723517038133731887338407189719511622662176727675373276651903807414909099441",
			"4168783608814932154536427934509895782246573715297911553964171371032945126671",
			"13891744915211034074451795021214165905772212241412891944830863846330766296736",
			"7937318970632701341203597196594272556916396164729705624521405069090520231616",
		},
		{
			-1, 1
		},
#if 0
		"13935988690119858555859723246779983032776768734325368709483973764294679676288 11103798494820748099527404339212658018601471515726261586464315054773579696226 8269576445442690838535980024167351791936352345724421614603369781441607794600 13354305407959090072726767749948715594166920939911867453711190606712455089306 11716754217617460302661193664575732761864016765151231423150245110906749536607 7729687816379319594108027529430724803285604293295766893032595449695780197922 13653964919137329928117176177966196509784862052610555758169065370283877591892 7116065892814627416819773460706997798054061654857442690703456886745403364820 15362798169581887105516891174389160237842443528684968714530077253285279265859 12620974470372210751490614470864622396139473762890526227333415568771040281183 15573296285785345620476983024059240851907011787481576804558527044472178152645 6966627880078199748567047058460482563613292011164226106423489555786390819693"
#else
		"8118772341496577043438385328606447626730215814727396173233264007541007797690 "
		"6742571767760762192519140673058087976840103832045324348366170860928670686713 "
		"9727912590495366720378364920530546614235713408261568635512172059018197267630 "
		"10180700148605185348549931182990442059136187839792856455707820203302941578832 "
		"5054507763444412917986776641611331046146804026682679569910978464879371792565 "
		"6917005519826733659554708445125877487590687705432214234949972860245110398023 "
		"10448556317747236258066222816126375978842661908560317699736569642190930635294 "
		"1516980358051268127904344653343215863076753141133525905743113718749531324025 "
		"9794836735385959178744195210089532061310424844916928682580569566332541022353 "
		"9375574834170998962484906689780052970915033987453510324648351251071086068423 "
		"710778048594563655498360873129325895716179849942646859397874562033386335205 "
		"10688745994254573144943003027511098295097561129365638275727908595677791826005"
#endif
	},
};

CYBOZU_TEST_AUTO(size)
{
	CYBOZU_TEST_EQUAL(sizeof(Fp), 32u);
	CYBOZU_TEST_EQUAL(sizeof(Fp2), sizeof(Fp) * 2);
	CYBOZU_TEST_EQUAL(sizeof(Fp6), sizeof(Fp) * 6);
	CYBOZU_TEST_EQUAL(sizeof(Fp12), sizeof(Fp) * 12);
	CYBOZU_TEST_EQUAL(sizeof(G1), sizeof(Fp) * 3);
	CYBOZU_TEST_EQUAL(sizeof(G2), sizeof(Fp2) * 3);
}
CYBOZU_TEST_AUTO(naive)
{
	const TestSet& ts = g_testSetTbl[0];
	BN::init(ts.cp, g_mode);
	G1 P(ts.g1.a, ts.g1.b);
	G2 Q(Fp2(ts.g2.aa, ts.g2.ab), Fp2(ts.g2.ba, ts.g2.bb));
	Fp12 e1;
	BN::pairing(e1, Q, P);
	Fp12 e2;
	{
		std::stringstream ss(ts.e);
		ss >> e2;
//		mpz_class x = BN::param.z;
//		x = 2 * x * (6 * x * x + 3 * x + 1);
//		Fp12::pow(e1, e1, x);
	}
	CYBOZU_TEST_EQUAL(e1, e2);
	/*
		ate-pairing
		miller loop : 700Kclk
		final exp   : 460Kclk
	*/
	CYBOZU_BENCH("pairing", BN::pairing, e1, Q, P); // 2.4Mclk
	CYBOZU_BENCH("finalExp", BN::finalExp, e1, e1); // 1.3Mclk
}

CYBOZU_TEST_AUTO(HashMapToG1)
{
	mcl::bn::HashMapToG1<Fp> hash;

	for (int i = 0; i < 10; i++) {
		G1 g;
		hash.calc(g, i, 2);
		std::cout << i << ':' << g << std::endl;
	}
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
