#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm> // for std::find

using namespace std;

// ANSI escape codes for colored text
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define BOLD    "\033[1m"

class SlabAllocator {
private:
    struct Slab {
        vector<void*> freeObjects;
        vector<void*> allocatedObjects;
    };

    unordered_map<size_t, Slab*> slabs;

public:
    ~SlabAllocator() {
        // Free all allocated memory to prevent leaks
        for (auto& pair : slabs) {
            for (auto& allocatedObj : pair.second->allocatedObjects) {
                ::operator delete(allocatedObj);
            }
            for (auto& freeObj : pair.second->freeObjects) {
                ::operator delete(freeObj);
            }
            delete pair.second;
        }
    }

    void* allocate(size_t objectSize) {
        Slab* slab = getOrCreateSlab(objectSize);

        if (!slab->freeObjects.empty()) {
            void* obj = slab->freeObjects.back();
            slab->freeObjects.pop_back();
            slab->allocatedObjects.push_back(obj);
            return obj;
        }

        void* newObject = ::operator new(objectSize);
        slab->allocatedObjects.push_back(newObject);
        return newObject;
    }

    void deallocate(void* obj, size_t objectSize) {
        Slab* slab = getSlab(objectSize);
        if (!slab) {
            cout << RED << "Error: Attempting to deallocate from a non-existent slab." << RESET << "\n";
            return;
        }

        auto it = find(slab->allocatedObjects.begin(), slab->allocatedObjects.end(), obj);
        if (it != slab->allocatedObjects.end()) {
            slab->allocatedObjects.erase(it);
            slab->freeObjects.push_back(obj);
        } else {
            cout << YELLOW << "Warning: Attempted to deallocate unmanaged object at address: " << obj << RESET << "\n";
            ::operator delete(obj);
        }
    }

    void printStatus(size_t objectSize) {
        Slab* slab = getSlab(objectSize);
        if (!slab) {
            cout << BLUE << "No slabs allocated yet for this size (" << objectSize << " bytes)." << RESET << "\n";
            return;
        }

        cout << "\n" << BOLD << "Slab Status for Size: " << objectSize << " bytes" << RESET << "\n";
        cout << "-------------------------------\n";
        cout << "Allocated objects: " << slab->allocatedObjects.size() << "\n";
        cout << "Free objects:      " << slab->freeObjects.size() << "\n";
        cout << "-------------------------------\n";

        cout << "\nIn real memory management systems, slab allocators help efficiently manage\n";
        cout << "fixed-size memory chunks, reducing fragmentation and speeding up allocations.\n";
    }

    void simulateOneCycle(size_t objectSize, vector<void*>& allocatedObjects) {
        cout << "\n" << BLUE << "Simulating Kernel Memory Operation (1 Cycle)..." << RESET << "\n";
        cout << "-------------------------------------------------\n";

        cout << GREEN << "Allocating memory..." << RESET << "\n";
        void* obj = allocate(objectSize);
        allocatedObjects.push_back(obj);
        cout << GREEN << "Allocated at address: " << obj << RESET << "\n";
        
        cout << "\n" << RED << "Deallocating memory..." << RESET << "\n";
        if (!allocatedObjects.empty()) {
            void* deallocObj = allocatedObjects.back();
            allocatedObjects.pop_back();
            deallocate(deallocObj, objectSize);
            cout << RED << "Deallocated address: " << deallocObj << RESET << "\n";
        }

        printStatus(objectSize);
    }

    void printErrorTheory(const string& errorMessage) {
        cout << "\n" << RED << "Error Theory: " << errorMessage << RESET << "\n";
        cout << "-------------------------------------------------\n";
        cout << "In memory management, errors like this often happen when:\n";
        cout << "- Trying to free memory not allocated by the allocator\n";
        cout << "- Accessing invalid or already freed memory\n";
        cout << "- Using incorrect object size during deallocation\n";
        cout << "Such mistakes can cause system crashes, leaks, or instability.\n";
    }

private:
    Slab* getOrCreateSlab(size_t objectSize) {
        if (slabs.find(objectSize) == slabs.end()) {
            slabs[objectSize] = new Slab();
        }
        return slabs[objectSize];
    }

    Slab* getSlab(size_t objectSize) {
        auto it = slabs.find(objectSize);
        return (it != slabs.end()) ? it->second : nullptr;
    }
};

void menu() {
    cout << "\n" << BOLD << BLUE << "Slab Allocator Simulation Menu" << RESET << "\n";
    cout << "=================================\n";
    cout << "1. Allocate Object\n";
    cout << "2. Deallocate Last Allocated Object\n";
    cout << "3. Print Slab Status\n";
    cout << "4. Simulate One Allocation + Deallocation Cycle\n";
    cout << "5. Exit\n";
    cout << "Enter your choice: ";
}

int main() {
    size_t objectSize;
    cout << GREEN << "Enter object size for the slab allocator (in bytes): " << RESET;
    cin >> objectSize;

    SlabAllocator allocator;
    vector<void*> allocatedObjects;

    while (true) {
        menu();
        int choice;
        cin >> choice;

        switch (choice) {
            case 1: {
                void* obj = allocator.allocate(objectSize);
                allocatedObjects.push_back(obj);
                cout << GREEN << "Object allocated at address: " << obj << RESET << "\n";
                break;
            }
            case 2: {
                if (allocatedObjects.empty()) {
                    cout << YELLOW << "No objects to deallocate." << RESET << "\n";
                    allocator.printErrorTheory("Deallocation attempted with no active objects");
                } else {
                    void* obj = allocatedObjects.back();
                    allocatedObjects.pop_back();
                    allocator.deallocate(obj, objectSize);
                    cout << GREEN << "Object deallocated at address: " << obj << RESET << "\n";
                }
                break;
            }
            case 3:
                allocator.printStatus(objectSize);
                break;
            case 4:
                allocator.simulateOneCycle(objectSize, allocatedObjects);
                break;
            case 5:
                cout << BLUE << "Exiting. Thank you!" << RESET << "\n";
                return 0;
            default:
                cout << RED << "Invalid choice. Please try again." << RESET << "\n";
                allocator.printErrorTheory("Invalid menu choice entered");
        }
    }

    return 0;
}
