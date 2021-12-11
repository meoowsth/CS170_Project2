#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <ctime>
using namespace std;

//global variables
vector<double> inputData;

//instance constructor, an instance is constructed for each row(each input)
struct instance{
    int type;
    vector<double> features;

    instance(int Type, int columnSize) {
        type = Type;
        addFeaturesToInstance(++columnSize);
    }
    //adding all features to its corresponding instance
    void addFeaturesToInstance(int columnSize) {
        for (int i = columnSize; i < inputData.size(); ++i) {
            if (inputData[i] == 1 || inputData[i] == 2) { //skipping the type
                break;
            } else {
                features.push_back(inputData[i]);
            }
        }
    }
};

//making all data into instances with feature vector
void convertToInstance(vector<instance> &listofData) {
    for (int i = 0; i < inputData.size(); ++i) {
        if (inputData[i] == 1) {
            instance newInstance(1, i);
            listofData.push_back(newInstance);
        } else if (inputData[i] == 2) {
            instance newInstance(2, i);
            listofData.push_back(newInstance);
        }
    }
}

//finding the index of the best feature to add
int getBestIndex(const vector<instance> &listofData, vector<int> &featureSet, int featureToAdd, int itself, double curBest) {
    int curBestIndex;
    for (int i = 0; i < listofData.size(); ++i) {
        double dist = 0;
        if (itself != i) {
            for (int j : featureSet) {
                dist += pow( listofData[itself].features[j] - listofData[i].features[j], 2); //euclidean distance
            }
            dist += pow( listofData[itself].features[featureToAdd] - listofData[i].features[featureToAdd], 2);
            dist = sqrt(dist);

            if (dist < curBest) {
                curBest = dist;
                curBestIndex = i;
            }
        }
    }
    return curBestIndex;
}

double leaveOneOut(vector<instance> listofData, vector<int> featureSet, int featureToAdd) {
    int correctCnt = 0;

    for (int i = 0; i < listofData.size(); ++i) {
        double curBest = 10000;
        int curBestIndex = getBestIndex(listofData, featureSet, featureToAdd, i, curBest);
        if (listofData[i].type == listofData[curBestIndex].type) {
            correctCnt++;
        }
    }
    return 1.0 * correctCnt / listofData.size();
}

int getBestIndexBack(const vector<instance> &listofData, vector<int> &featureSetTemp, int itself, double curBest) {
    int curBestIndex;
    for (int i = 0; i < listofData.size(); ++i) {
        double dist = 0;
        if (itself != i) {
            for (int j : featureSetTemp) {
                dist += pow( listofData[itself].features[j] - listofData[i].features[j], 2);
            }
            dist = sqrt(dist);
            if (dist < curBest) {
                curBest = dist;
                curBestIndex = i;
            }
        }
    }
    return curBestIndex;
}

double leaveOneOutBackward(vector<instance> listofData, vector<int> featureSet, int featureToRemove) {
    int correctCnt = 0;
    vector<int> featureSetTemp;

    for (int & i : featureSet) {
        if (i != featureToRemove) {
            featureSetTemp.push_back(i);
        }
    }
    for (int i = 0; i < listofData.size(); ++i) {
        double curBest = 10000;
        int  curBestIndex = getBestIndexBack(listofData, featureSetTemp, i, curBest);
        if (listofData[i].type == listofData[curBestIndex].type) {
            correctCnt++;
        }
    }
    return 1.0 * correctCnt / listofData.size() ;
}

void update(vector<instance> &listofData, vector<int> &featureSet, int &featureToAdd, double &curBest) {
    for (int i = 0; i < listofData[0].features.size(); ++i) {
        if (find(featureSet.begin(), featureSet.end(), i) == featureSet.end()) {
            double accuracy = leaveOneOut(listofData, featureSet, i);

            if (accuracy > curBest) {
                curBest = accuracy;
                featureToAdd = i;
            }
        }
    }
}

void forwardSelection(vector<instance> listofData) {
    vector<int> featureSet;
    vector<int> bestSubset;
    double best = 0;
    bool decrease = false;
    cout << "Starting forward selection." << endl;

    for (int i = 0; i < listofData[0].features.size(); ++i) {
        int featureToAdd = 0;
        double curBest = 0;

        update(listofData, featureSet, featureToAdd, curBest);
        featureSet.push_back(featureToAdd);

        if (curBest > best) {
            best = curBest;
            bestSubset = featureSet;
        } else if (curBest < best && !decrease) {
            cout << "(Warning, Accuracy has decreased! Continuing search in case of local maxima)" << endl;
            decrease = true;
        }

        cout << "Feature set {";
        for (int j = 0; j < featureSet.size() - 1; j++) {
            cout << featureSet[j] + 1 << ", ";
        }
        cout << featureSet[featureSet.size() - 1] + 1;
        cout << "} was the best, accuracy = " << setprecision(4) << curBest * 100 << "%" << endl << endl;
    }

    cout << "Finished search with forward selection. The best feature subset is {";
    for (int i = 0; i < bestSubset.size() - 1; ++i) {
        cout << bestSubset[i] + 1 << ", ";
    }
    cout << bestSubset[bestSubset.size() - 1] + 1 << "}, which has an accuracy = " << setprecision(3) << best * 100 << "%" << endl << endl;
}

void backwardHelper(vector<instance> &listofData, vector<int> &featureSet, int &featureToRemove, double &curBest) {
    for (int i = 0; i < listofData[0].features.size(); ++i) {
        if (find(featureSet.begin(), featureSet.end(), i) != featureSet.end() && featureSet.size() > 1) {
            double accuracy = leaveOneOutBackward(listofData, featureSet, i);
            cout << "Using feature set {";

            for (int j = 0; j < featureSet.size() - 2; j++) {
                if (featureSet[j] != i) {
                    cout << featureSet[j] + 1 << ", ";
                }
            }
            if (featureSet[featureSet.size() - 2] == i) {
                cout << featureSet[featureSet.size() - 1] + 1;
            } else if (featureSet[featureSet.size() - 1] == i) {
                cout << featureSet[featureSet.size() - 2] + 1;
            } else {
                cout << featureSet[featureSet.size() - 2] + 1 << ", ";
                cout << featureSet[featureSet.size() - 1] + 1;
            }
            cout << "} accuracy is " << setprecision(4) << accuracy * 100 << "%" << endl;

            if (accuracy > curBest) {
                curBest = accuracy;
                featureToRemove = i;
            }
        }
    }
}

void backwardElimination(vector<instance> listofData) {
    vector<int> featureSet;
    vector<int> bestSubset;
    double best = 0;
    bool decrease = false;
    bool outputCheck = false;
    cout << "Starting backward elimination." << endl << endl;

    featureSet.reserve(listofData[0].features.size());
    for (int i = 0; i < listofData[0].features.size(); ++i) {
        featureSet.push_back(i);
    }

    cout << "Starting feature set: ";
    for (int i : featureSet) {
        cout << i + 1 << " ";
    }
    cout << endl << endl;

    for (int i = 0; i < listofData[0].features.size(); ++i) {
        int featureToRemove = 0;
        double curBest = 0;

        backwardHelper(listofData, featureSet, featureToRemove, curBest);
        cout << endl;

        for (int i = 0; i < featureSet.size(); ++i) {
            if (featureSet[i] == featureToRemove) {
                featureSet.erase(featureSet.begin() + i);
                break;
            }
        }
        if (curBest > best) {
            best = curBest;
            bestSubset = featureSet;
        } else if (curBest < best && !decrease) {
            cout << "(Warning, Accuracy has decreased! Continuing search in case of local maxima)" << endl << endl;
            decrease = true;
        }
        if (featureSet.size() != 1 || (featureSet.size() == 1 && !outputCheck)) {
            if (featureSet.size() == 1) {
                outputCheck = true;
            }
            cout << "Feature set {";
            for (int x = 0; x < featureSet.size() - 1; x++) {
                cout << featureSet[x] + 1 << ", ";
            }
            cout << featureSet[featureSet.size() - 1] + 1;
            cout << "} was the best, accuracy is " << setprecision(10) << curBest * 100 << "%" << endl;
        }

    }

    cout << "Finished search with backward elimination. The best feature subset is {";
    for (int i = 0; i < bestSubset.size() - 1; ++i) {
        cout << bestSubset[i] + 1 << ", ";
    }
    cout << bestSubset[bestSubset.size() - 1] + 1 << "} which has an accuracy of " << best * 100 << "%\n\n";

}

int main() {
    vector<instance> listofData;
    string inputFileName;
    ifstream inputFile;
    double temp = 0;
    int userInput;

    cout << "Please choose dataset (1 or 2): " << endl;
    cout << "  1. Use a small dataset" << endl << "  2. Use a large dataset" << endl;
    cin >> userInput;

    while(userInput){
        if (userInput == 1) {
            inputFileName = "Small37.txt";
            break;
        } else if (userInput == 2) {
            inputFileName = "Large74.txt";
            break;
        } else {
            cout << "Please type a valid number (1 or 2)." << endl;
            cin >> userInput;
        }
    }
    inputFile.open(inputFileName);
    if (!inputFile) {
        cout << "Unable to open file." << endl;
        exit(1);
    }

    while (inputFile >> temp) {
        inputData.push_back(temp); //adding all data to the global dataInput vector
    }

    convertToInstance(listofData); //convert elements in dataInput to instances and add to vector dada

    cout << "Please select the algorithm you want to run (number 1 or 2): " << endl;
    cout << "  1. Forward Selection" << endl << "  2. Backward Elimination" << endl;
    cin >> userInput;


    while(true){
        clock_t timeConsumed = clock();
        if (userInput == 1) {
            cout << endl << "This dataset has " << listofData[0].features.size() << " features (not including the class attribute), with " << listofData.size() << " instances." << endl << endl;
            forwardSelection(listofData);
            timeConsumed = clock() - timeConsumed;
            cout << "Time consumed using forward selection: " << 1.0 * (timeConsumed) / CLOCKS_PER_SEC / 60 << " minutes" << endl;
            break;
        } else if (userInput == 2) {
            cout << endl << "This dataset has " << listofData[0].features.size() << " features (not including the class attribute), with " << listofData.size() << " instances." << endl << endl;
            backwardElimination(listofData);
            timeConsumed = clock() - timeConsumed;
            cout << "Time consumed using backward elimination: " << 1.0 * (timeConsumed) / CLOCKS_PER_SEC / 60 << " minutes" << endl;
            break;
        } else {
            cout << "Please select a valid input (1 or 2)." << endl;
            cin >> userInput;
        }
    }
    inputFile.close();

    return 0;
}