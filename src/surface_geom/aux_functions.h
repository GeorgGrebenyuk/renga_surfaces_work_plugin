#pragma once
#include <string>
#include <vector>

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}
void del_from_list_by_edge(std::vector<std::vector<int>>* current_collection, std::vector<int>* what_del)
{
    (*current_collection).erase(std::remove((*current_collection).begin(), (*current_collection).end(), *what_del), (*current_collection).end());
}
void insert_edge_if_not_in(std::vector<std::vector<int>>* current_collection, std::vector<int>* new_item)
{
    bool need_add = false;
    if ((*current_collection).empty()) need_add = true;
    else
    {
        std::vector<int> other_var{ new_item->at(1),new_item->at(0) };
        bool is_var_1 = (std::find((*current_collection).begin(), (*current_collection).end(), (*new_item)) != (*current_collection).end());
        bool is_var_2 = (std::find((*current_collection).begin(), (*current_collection).end(), &other_var) != (*current_collection).end());
        if (!is_var_1 && !is_var_2) need_add = true;
        else
        {
            need_add = false;
            del_from_list_by_edge(current_collection, new_item);
            del_from_list_by_edge(current_collection, &other_var);
        }
    }
    if (need_add == true)
    {
        (*current_collection).push_back((*new_item));
    }

}
void start_loop(std::vector<std::vector<int>>* current_collection, std::vector<std::vector<int>>* outer_collection, int* point_find)
{
    for (std::vector<int> one_edge : (*current_collection))
    {
        if (one_edge.at(0) == *point_find | one_edge.at(1) == *point_find)
        {
            (*outer_collection).push_back(one_edge);
            if (one_edge.at(0) == *point_find) *point_find = one_edge.at(1);
            else *point_find = one_edge.at(0);
            del_from_list_by_edge(current_collection, &one_edge);
            break;
        }

    }
}
void get_outer_counter(std::vector<std::vector<int>>* current_collection, std::vector<std::vector<int>>* outer_collection)
{
    //‘иксируем начало отбора - одну из "внешних" граней, с удалением ее из первого списка и сохранением во второй - целевой
    std::vector<int> start_edge = (*current_collection).at(0);
    del_from_list_by_edge(current_collection, &start_edge);
    (*outer_collection).push_back(start_edge);
    //‘иксируем индекс точки, к котором мы должны прийти по мере обхода всех внешних ребер
    int end_index = start_edge.at(0);
    //‘иксируем индекс точки, равный которому будем искать в дальнейшем обходе всех оставшихс€ внешних ребер. —ћ≈ЌЌџ… ѕј–јћ≈“–
    int start_going = start_edge.at(1);

    do {
        start_loop(current_collection, outer_collection, &start_going);
    } while (start_going == end_index);
    //проверить емкость нового массива и старого: если в старом что-то осталось - гл€нуть любую точку на предмет попадани€ в полигон нового 
    //- если в нем, значит это внешн€€ граница. ¬ противном случае  - внутренн€€ .... хот€ бл€ть, есть еще вариант "островка" после внутреннего озера 
    //в общем считаем пока как будто нет границ внутренних
}

