#include "OptimizationProblem.h"
#include <math.h>
#include <limits>

using std::vector ;

/*The functionality for this class was converted to C++ from a Java library in Alrecenk's structure from motion repository.
https://github.com/Alrecenk/StructureFromMotion/blob/master/src/Utility.java
*/


//Returns the gradient calculated numerically using the error function
std::vector<double> OptimizationProblem::numericalGradient(std::vector<double> x, double epsilon){
    vector<double> gradient ;
    for(int k=0;k<x.size();k++){
        x[k] += epsilon;
        double xplus = this->error(x);
        x[k] -= 2*epsilon;
        double xminus = this->error(x);
        x[k] += epsilon;
        gradient.push_back((xplus-xminus)/(2*epsilon));
    }
    return gradient ;
}

std::vector<double> OptimizationProblem::minimizeByLBFGS(std::vector<double> x0, int m, int maxiter, int stepiter, double tolerance, double min_improvement){
    int iter = 0;
    double error = this->error(x0);
    double lasterror = std::numeric_limits<float>::max();
    vector<double> gradient = this->gradient(x0);
    //Console.Out.WriteLine("Start error: " + error + "  gradient norm :" + norm(gradient)) ;
    //error = feval(fun,x.p,1);
    //gradient = feval(fun,x.p,2);
    vector<double> g = gradient;
    //preallocate arrays
    int k = 0, j;
    vector<vector<double>> s ;
    vector<double> rho ;
    vector<vector<double>> y ;
    vector<double> nw ;
    vector<double> r, q;
    double B;

    for(int k=0;k<m;k++){
        s.push_back(vector<double>());
        rho.push_back(0);
        y.push_back(vector<double>());
        nw.push_back(0);
    }

    //quit when acceptable accuracy reached or iterations exceeded
    while (iter < maxiter && norm(gradient) > tolerance && lasterror-error > min_improvement*error){
        lasterror = error;
        iter = iter + 1;
        k = k + 1;
        g = gradient;
        //%initial pass just use gradient descent
        if (k == 1 || x0.size() ==1){
            r = g;
        }else{
            //two loop formula
            q = g;
            int i = k - 1;
            while (i >= k - m && i > 0){
                j = i % m; //%index into array operating as a fixed size queue
                nw[j] = rho[j] * dot(s[j], q);
                q = subtract(q, scale(y[j], nw[j]));
                i = i - 1;
            }
            j = (k - 1) % m;
            r = scale(q, dot(s[j], y[j]) / dot(y[j], y[j]));// % gamma,k * q = H0k q
            i = fmax(k - m, 1);
            while (i <= k - 1){
                j = i % m; //%index into array operating as a fixed size queue
                B = rho[j] * dot(y[j], r);

                r = add(r, scale(s[j], nw[j] - B));
                i = i + 1;
            }
        }
        //% get step size
        // alfa = StepSize(fun, x, -r, 1,struct('c1',10^-4,'c2',.9,'maxit',100)) ;
        double alfa = stepSize(x0, scale(r, -1), 1, stepiter, .1, .9);

        //%apply step and update arrays
        j = k % m;
        s[j] = scale(r, -alfa);
        /*
        if (containsnan(s[j]) || alfa <= 0){
            System.err.println("Invalid exit condition in LBFGS!" + alfa);
            return x0;
        }*/
        x0 = add(x0, s[j]);
        gradient = this->gradient(x0);
        error = this->error(x0);
        y[j] = subtract(gradient, g);
        rho[j] = 1.0 / dot(y[j], s[j]);
    }
    return x0;


}

std::vector<double> OptimizationProblem::minimumByGradientDescent(std::vector<double> x0, double tolerance, int maxiter){
    vector<double> x = x0 ;
    vector<double> gradient = this->gradient(x0) ;
    int iteration = 0 ;
    while(dot(gradient,gradient) > tolerance*tolerance && iteration < maxiter){
        iteration++ ;
        //calculate step-size in direction of negative gradient
        double alpha = stepSize(x, scale(gradient,-1), 1, 500, 0.1, 0.9) ;
        x = add( x, scale(gradient, -alpha)) ; // apply step
        gradient = this->gradient(x) ; // get new gradient
    }
    return x ;

}

double OptimizationProblem::stepSize(std::vector<double> x0, std::vector<double> d, double alpha, int maxit, double c1, double c2){
    //get error and gradient at starting point  
    double fx0 = this->error(x0);
    double gx0 = dot(this->gradient(x0), d);
    //bound the solution
    double alphaL = 0;
    double alphaR = 1000;
    for (int iter = 1; iter <= maxit; iter++){
        vector<double> xp = add(x0, scale(d, alpha)); // get the point at this alpha
        double erroralpha = this->error(xp); //get the error at that point
        if (erroralpha >= fx0 + alpha * c1 * gx0)	{ // if error is not sufficiently reduced
            alphaR = alpha;//move halfway between current alpha and lower alpha
            alpha = (alphaL + alphaR)/2.0;
        }else{//if error is sufficiently decreased 
            double slopealpha = dot(this->gradient(xp), d); // then get slope along search direction
            if (slopealpha <= c2 * abs(gx0)){ // if slope sufficiently closer to 0
                return alpha;//then this is an acceptable point
            }else if ( slopealpha >= c2 * gx0) { // if slope is too steep and positive then go to the left
                alphaR = alpha;//move halfway between current alpha and lower alpha
                alpha = (alphaL+ alphaR)/2;
            }else{//if slope is too steep and negative then go to the right of this alpha
                alphaL = alpha;//move halfway between current alpha and upper alpha
                alpha = (alphaL+ alphaR)/2;
            }
        }
    }
    //if ran out of iterations then return the best thing we got
    return alpha;
}

std::vector<double> OptimizationProblem::add(std::vector<double> a, std::vector<double> b){
    std::vector<double> c ;
    for(int k=0;k<a.size();k++){
        c.push_back(a[k] + b[k]);
    }
    return c ;
}
std::vector<double> OptimizationProblem::subtract(std::vector<double> a, std::vector<double> b){
    std::vector<double> c ;
    for(int k=0;k<a.size();k++){
        c.push_back(a[k] - b[k]);
    }
    return c ;
}
double OptimizationProblem::dot(std::vector<double> a, std::vector<double> b){
    double dot = 0 ;
    for(int k=0;k<a.size();k++){
        dot += a[k] * b[k] ;
    }
    return dot ;
}
std::vector<double> OptimizationProblem::scale(std::vector<double> a, double s){
    std::vector<double> c ;
    for(int k=0;k<a.size();k++){
        c.push_back(a[k] *s);
    }
    return c ;
}

double OptimizationProblem::norm(std::vector<double> a){
    return sqrt(dot(a,a));
}