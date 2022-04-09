#ifndef _OPTIMIZATION_PROBLEM_H_
#define _OPTIMIZATION_PROBLEM_H_ 1

#include <vector>

class OptimizationProblem {
  public:

    // virtual destructor makes sure the right destructor is called on child classes
    virtual ~OptimizationProblem();

    // Return the current x for this object
    virtual std::vector<double> getX();

    // Set this object to a given x
    virtual void setX(std::vector<double> x);

    // Returns the error to be minimized for the given input
    virtual double error(std::vector<double> x);

    // Returns the gradient of error about a given input
    virtual std::vector<double> gradient(std::vector<double> x);

    //Returns the gradient calculated numerically using the error function
    std::vector<double> numericalGradient(std::vector<double> x, double epsilon);


    //Returns the local minimum by using the LBFGS method starting from x0
	//maxiter is the maximum iterations of LBFGS to perform
	//stepiter is the maximum iterations to perform in line search
	//tolerance maximum gradient norm of error used an an exit condition
	//min_improvement is a fraction of the error removed in this step considered as an exit condition
    std::vector<double> minimizeByLBFGS(std::vector<double> x0, int m, int maxiter, int stepiter, double tolerance, double min_improvement);

    //starting from w0 searches for a weight vector using gradient descent
	//and Wolfe condition line-search until the gradient magnitude is below tolerance
	//or a maximum number of iterations is reached
    std::vector<double> minimumByGradientDescent(std::vector<double> x0, double tolerance, int maxiter);

    //Performs a binary search to satisfy the Wolfe conditions
	//returns alpha where next x = x0 + alpha*d 
	//guarantees convergence as long as search direction is bounded away from being orthogonal with gradient
	//x0 is starting point, d is search direction, alpha is starting step size, maxit is max iterations
	//c1 and c2 are the constants of the Wolfe conditions (0.1 and 0.9 can work)
    double stepSize(std::vector<double> x0, std::vector<double> d, double alpha, int maxit, double c1, double c2);

    static std::vector<double> add(std::vector<double> a, std::vector<double> b);
    static std::vector<double> subtract(std::vector<double> a, std::vector<double> b);
    static double dot(std::vector<double> a, std::vector<double> b);
    static std::vector<double> scale(std::vector<double> a, double s);
    static double norm(std::vector<double> a);

};

#endif // #ifndef _OPTIMIZATION_PROBLEM_H_