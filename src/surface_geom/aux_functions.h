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
    //int where_is_element = 0;
    std::vector<std::vector<int>> new_list;
    for (std::vector<int> one_record : (*current_collection))
    {
        if ((one_record.at(0) == (*what_del).at(0) && one_record.at(1) == (*what_del).at(1)) |
            (one_record.at(0) == (*what_del).at(1) && one_record.at(1) == (*what_del).at(0)))
        {
            continue;
        }
        else {
            new_list.push_back(one_record);
        }
    }
    (*current_collection) = new_list;
}
bool is_element_in_list(std::vector<std::vector<int>>* current_collection, std::vector<int>* what_find)
{
    bool is_el = false;
    for (std::vector<int> one_record : (*current_collection))
    {
        if ((one_record.at(0) == (*what_find).at(0) && one_record.at(1) == (*what_find).at(1)) | 
            (one_record.at(0) == (*what_find).at(1) && one_record.at(1) == (*what_find).at(0)))
        {
            is_el = true;
            break;
        }
    }
    return is_el;
}
void insert_edge_if_not_in(std::vector<std::vector<int>>* current_collection, std::vector<int>* new_item)
{
    bool need_add = false;
    if ((*current_collection).empty()) need_add = true;
    else
    {
        std::vector<int> other_var{ new_item->at(1),new_item->at(0) };
        bool is_var_1 = is_element_in_list(current_collection, new_item);
        //bool is_var_2 = is_element_in_list(current_collection, &other_var);
        //Если в списке нет ни первого ни второго вхождения - то добавляем
        if (is_var_1 == false) need_add = true;
        //Если в списке есть хотя бы одно вхождение (вариация) грани, значит, необходимо ее удалить из перечня, а новую не вносить
        else
        {
            need_add = false;
            del_from_list_by_edge(current_collection, new_item);
            //del_from_list_by_edge(current_collection, &other_var);
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
        if (one_edge.at(0) == (*point_find) | one_edge.at(1) == (*point_find))
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
    std::cout << "current indexes" << std::endl;
    for (std::vector<int> one_record : (*current_collection))
    {
        std::cout << one_record.at(0) << " " << one_record.at(1) << std::endl;
    }
    //Фиксируем начало отбора - одну из "внешних" граней, с удалением ее из первого списка и сохранением во второй - целевой
    std::vector<int> start_edge = (*current_collection).at(0);
    del_from_list_by_edge(current_collection, &start_edge);
    (*outer_collection).push_back(start_edge);
    //Фиксируем индекс точки, к котором мы должны прийти по мере обхода всех внешних ребер
    int end_index = start_edge.at(0);
    //Фиксируем индекс точки, равный которому будем искать в дальнейшем обходе всех оставшихся внешних ребер. СМЕННЫЙ ПАРАМЕТР
    int start_going = start_edge.at(1);
    int last_index = 0;

    do {
        start_loop(current_collection, outer_collection, &start_going);
    } while (start_going == end_index);
    //проверить емкость нового массива и старого: если в старом что-то осталось - глянуть любую точку на предмет попадания в полигон нового 
    //- если в нем, значит это внешняя граница. В противном случае  - внутренняя .... хотя блять, есть еще вариант "островка" после внутреннего озера 
    //в общем считаем пока как будто нет границ внутренних
}
void sort_edges_to_unique(std::vector<std::vector<int>> *old_collection, std::vector<std::vector<int>>* new_collection)
{
    //Перебираем текущий список ребер и заносим в список на удаление те, которые уже встречаются в данном списке
    std::vector<std::vector<int>> to_delete;
    for (std::vector<int> one_record : *old_collection)
    {
        int counter_edge = 0;
        std::vector<int> other_var{ one_record.at(1),one_record.at(0) };
        for (std::vector<int> one_record_2 : *old_collection)
        {
            //если есть ребро с противоположной конфигурацией
            if (one_record_2.at(0) == one_record.at(1) && one_record_2.at(1) == one_record.at(0))
            {
                to_delete.push_back(one_record);
                to_delete.push_back(other_var);
                break;
            }
            else if (one_record_2.at(0) == one_record.at(0) && one_record_2.at(1) == one_record.at(1))
            {
                counter_edge++;
            }
        }
        //Если было найдено 2 таких же ребра
        if (counter_edge == 2) to_delete.push_back(one_record);
    }
    sort((to_delete).begin(), (to_delete).end());
    (to_delete).erase(unique((to_delete).begin(), (to_delete).end()), (to_delete).end());
    //Создаем новый список, теперь уже с нужными значениями без ненужных

    for (std::vector<int> one_record : *old_collection)
    {
        //std::vector<int> other_var{ one_record.at(1),one_record.at(0) };
        bool need_add = true;
        for (std::vector<int> one_record_2 : to_delete)
        {
            //one_record_2.at(0) == one_record.at(0) && one_record_2.at(1) == one_record.at(1)
            if (one_record == one_record_2)
            {
                need_add = false;
            }
        }
        if (need_add) (*new_collection).push_back(one_record);
    }
    to_delete.clear();
}
