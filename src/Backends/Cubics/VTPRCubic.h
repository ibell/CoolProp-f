//
//  VTPRCubic.h
//  CoolProp
//
//  Created by Ian on 7/17/16.
//
//

#include "GeneralizedCubic.h"

#ifndef VTPRCubic_h
#define VTPRCubic_h

class VTPRCubic : public PengRobinson
{
private:
    UNIFAQ::UNIFAQMixture unifaq;
public:
    VTPRCubic(std::vector<double> Tc,
              std::vector<double> pc,
              std::vector<double> acentric,
              double R_u,
              const UNIFAQLibrary::UNIFAQParameterLibrary & lib
              )
    : PengRobinson(Tc, pc, acentric, R_u), unifaq(lib) {};
    
    VTPRCubic(double Tc,
              double pc,
              double acentric,
              double R_u,
              const UNIFAQLibrary::UNIFAQParameterLibrary & lib)
    : PengRobinson(std::vector<double>(1,Tc), std::vector<double>(1,pc), std::vector<double>(1,acentric), R_u), unifaq(lib) {};
    
    /// Get a reference to the managed UNIFAQ instance
    UNIFAQ::UNIFAQMixture &get_unifaq(){ return unifaq; }
    
    /// The attractive part in cubic EOS
    double a_alpha(double T, std::size_t i){
        return pow(1 + m_ii(i)*(1-sqrt(T/Tc[i])), 2);
    }
    /// Calculate the non-dimensionalized gE/RT term
    double gE_R_RT(){
        const std::vector<double> &z = unifaq.get_mole_fractions();
        double summer = 0;
        for (std::size_t i = 0; i < z.size(); ++i) {
            summer += z[i]*unifaq.ln_gamma_R(i);
        }
        return summer;
    }
    /// The co-volume for the i-th pure component
    double b_ii(std::size_t i){
        return 0.0778*R_u*Tc[i]/pc[i];
    }
    /// The attractive parameter for the i-th pure component
    double a_ii(std::size_t i){
        double a0 = 0.45724*pow(R_u*Tc[i], 2)/pc[i];
        double alpha = a_alpha(unifaq.get_temperature(), i);
        return a0*alpha;
    }
    double am_term(double tau, const std::vector<double> &x, std::size_t itau){
        if (itau == 0){
            set_temperature(T_r/tau);
            double _am,_bm; am_bm(_am, _bm);
            return _am;
        }
        else{
			double dtau = 0.01*tau;
			return (am_term(tau + dtau, x, itau - 1) - am_term(tau - dtau, x, itau - 1)) / (2 * dtau);
        }
    }
	double d_am_term_dxi(double tau, const std::vector<double> &x, std::size_t itau, std::size_t i, bool xN_independent)
	{
		throw CoolProp::NotImplementedError();
	}
	double d2_am_term_dxidxj(double tau, const std::vector<double> &x, std::size_t itau, std::size_t i, std::size_t j, bool xN_independent)
	{
		throw CoolProp::NotImplementedError();
	}
	double d3_am_term_dxidxjdxk(double tau, const std::vector<double> &x, std::size_t itau, std::size_t i, std::size_t j, std::size_t k, bool xN_independent)
	{
		throw CoolProp::NotImplementedError();
	}

    double bm_term(const std::vector<double> &x){
        double _am,_bm; am_bm(_am, _bm);
        return _bm;
    }
	double d_bm_term_dxi(const std::vector<double> &x, std::size_t i, bool xN_independent)
	{
		double summer = 0;
		if (xN_independent)
		{
			for (int j = N - 1; j >= 0; --j)
			{
				summer += x[j] * bij_term(i, j);
			}
			return 2 * summer;
		}
		else {
			for (int k = N - 2; k >= 0; --k)
			{
				summer += x[k] * (bij_term(i, k) - bij_term(k, N - 1));
			}
			return 2 * (summer + x[N - 1] * (bij_term(N - 1, i) - bij_term(N - 1, N - 1)));
		}
	}
	double d2_bm_term_dxidxj(const std::vector<double> &x, std::size_t i, std::size_t j, bool xN_independent)
	{
		if (xN_independent)
		{
			return 2 * bij_term(i, j);
		}
		else {
			return 2 * (bij_term(i, j) - bij_term(j, N - 1) - bij_term(N - 1, i) + bij_term(N - 1, N - 1));
		}
	}
	double d3_bm_term_dxidxjdxk(const std::vector<double> &x, std::size_t i, std::size_t j, std::size_t k, bool xN_independent)
	{
		return 0;
	}

    /// Calculate both am and bm because am and bm are dependent on each other
    void am_bm(double &am, double &bm){
        const std::vector<double> &z = unifaq.get_mole_fractions();
        double summeram = 0, summerbm = 0;
        for (std::size_t i = 0; i < z.size(); ++i){
            summeram += z[i]*a_ii(i)/b_ii(i);
            for (std::size_t j = 0; j < z.size(); ++j){
                summerbm += z[i]*z[j]* bij_term(i,j);
            }
        }
        bm = summerbm;
        am = bm*(summeram + R_u*unifaq.get_temperature()*gE_R_RT()/(-0.53087));
    };
	double bij_term(std::size_t i, std::size_t j)
	{
		return pow((pow(b_ii(i), 0.75) + pow(b_ii(j), 0.75)) / 2.0, 4.0 / 3.0);
	}
    void set_temperature(const double T){ unifaq.set_temperature(T); }
};

#endif /* VTPRCubic_h */
