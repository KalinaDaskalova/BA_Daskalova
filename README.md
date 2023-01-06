# BA_Daskalova

Topic:
Group Navigation with Computer-Controlled Companions Through an Immersive, Architectural Virtual Environment


Summary of the bachelor thesis:
The thesis consists of a city tour simulated in virtual reality. The user wears a head-mounted display and is immersed into the virtual world, where they
take part in a virtual city tour together with three other computer-controlled virtual humans and one virtual guide. The algorithm created for this thesis 
includes the movement of the virtual humans in a specific formation while considering the user. Audio, animations and facial expressions have also been 
specifically created for this thesis.

Disclaimer: This repository does not include the whole project, but only the code written for it by Kalina Daskalova.


Short desription of the code:
The main components of the structure of the project framework are the class of the virtual guide, called UVHGuide and the Interface IStates, 
that defines the basic functionalities of each of the group’s states. The group states are defined as follows: UMeetingPoint, UToPoI, UAtPoI, 
UApproachUser and UGoodbye. Based on different conditions and actions of the user, the states can change. The project also includes the classes of each 
of the virtual humans, where their behavior is defined. All other additional classes serve as triggers for different behavioral features of the 
virtual humans and the environment. 
